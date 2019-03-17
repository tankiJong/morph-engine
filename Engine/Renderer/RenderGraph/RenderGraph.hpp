#pragma once

#include "Engine/Core/common.hpp"

#include <map>
#include <functional>
#include <unordered_set>
#include "Engine/Renderer/RenderGraph/RenderEdge.hpp"
#include "Engine/Renderer/RenderGraph/RenderGraphResourceSet.hpp"

class RenderPass;
class RenderNode;
class RHIContext;
class RenderNodeContext;
class RenderPassExecutor;
class RenderNodeBuilder;

class RenderPassConstructor: public std::function<RenderPassExecutor(RenderNodeBuilder&, RenderNodeContext&)> {
  using BaseType = std::function<RenderPassExecutor(RenderNodeBuilder&, RenderNodeContext&)>;
public:
  RenderPassConstructor() = default;

  template<typename Lambda, 
  typename = std::enable_if_t<!std::is_same_v<std::decay_t<Lambda>, RenderPassConstructor> && !std::is_void_v<Lambda>>>
  RenderPassConstructor(Lambda&& lambda)
    : BaseType(std::forward<Lambda>(lambda)) {
    using RtnType = std::invoke_result_t<Lambda, RenderNodeBuilder&, RenderNodeContext&>;
    static_assert(std::is_same_v<void, std::invoke_result_t<RtnType, const RenderGraphResourceSet&, RHIContext&>>);
  };

  using BaseType::operator();

protected:
};

class RenderPassExecutor: 
  public std::function<void(const RenderGraphResourceSet&, RHIContext&)> {
  using BaseType = std::function<void(const RenderGraphResourceSet&, RHIContext&)>;
public:
  RenderPassExecutor() = default;

  template<typename Lambda, 
  typename = std::enable_if_t<!std::is_same_v<std::decay_t<Lambda>, RenderPassExecutor> && !std::is_void_v<Lambda>>>
  RenderPassExecutor(Lambda&& lambda)
    : BaseType(std::forward<Lambda>(lambda)) {
    static_assert(std::is_same_v<void, std::invoke_result_t<Lambda, const RenderGraphResourceSet&, RHIContext&>>);
  };

  using BaseType::operator();
protected:
};


class RenderGraph {
  friend class RenderNode;
public:

  RenderNode& defineNode(std::string_view name, RenderPassConstructor constructor);

  template<typename RenderPassType, typename ...Args>
  RenderNode& createNode(std::string_view name, Args ...args) {
    static_assert(std::is_base_of_v<RenderPass, RenderPassType>, "The type has to derive from RenderPass class");
    RenderPass* pass = allocatePass<RenderPassType>(args...);
    return addPass(name, pass);
  }

  void depend(RenderNode& blocker, RenderNode& blockee);

  void bind(std::string_view resourceInPass, std::string_view resource);

  void connect(RenderNode& fromNode, std::string_view fromNodeOutParam, RenderNode& toNode, std::string_view toNodeInParam);

  // void connect(RenderNode& fromNode, std::string_view fromNodeOutParam, std::string_view resName);
  //
  // void connect(std::string_view resName, RenderNode& toNode, std::string_view toNodeInParam);
 
  /**
   * Connect from a resource/param to another resource/param.
   * 
   * For resource: resource name;
   * For param: [pass-name].[param-name];
   *        
   */
  void connect(std::string_view fromRes, std::string_view toRes);

  template<typename T>
  void setParam(rhi_sptr_t<T> p, std::string_view name) {
    auto handle = mResourceSet.find(name);
    mResourceSet.set<T>(p, *handle, RenderGraphResourceHandle::OWN_EXTERNAL, true);
  }

  void setCustomBackBuffer(const Texture2::sptr_t& backbuffer) {
    mResourceSet.setBackbuffer(backbuffer);
  }

  void setOutputPass(RenderNode& node) { mOutputNode = &node; }

  bool execute();

  bool build();

  template<typename T>
  decltype(auto) declare(std::string name) {
    return mResourceSet.declare<T>(name);
  }

protected:

  RenderNode& addPass(std::string_view name, RenderPass* pass);

  template<typename RenderPassType, typename ...Args>
  RenderPass* allocatePass(Args ...args) {
    return new RenderPassType(args...);
  }

  void freePass(RenderPass* pass);

  std::map<std::string, RenderNode*, std::greater<>> mDefinedNodes;
  std::unordered_set<RenderEdge> mEdges;
  RenderNode* mOutputNode;
  RenderGraphResourceSet mResourceSet;

private:
  void topologySortVisitor(std::vector<RenderNode*>& sortedNodes, RenderNode* node) const;
};

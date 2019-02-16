#pragma once

#include "Engine/Core/common.hpp"

#include <map>
#include <functional>
#include <unordered_set>
#include "Engine/Renderer/RenderGraph/RenderEdge.hpp"

class RenderPass;
class RenderNode;
class RHIContext;
class RenderNodeContext;
class RenderPassExecutor;

class RenderPassConstructor: public std::function<RenderPassExecutor(RenderNodeContext&)> {
  using BaseType = std::function<RenderPassExecutor(RenderNodeContext&)>;
public:
  RenderPassConstructor() = default;

  template<typename Lambda, 
  typename = std::enable_if_t<!std::is_same_v<std::decay_t<Lambda>, RenderPassConstructor> && !std::is_void_v<Lambda>>>
  RenderPassConstructor(Lambda&& lambda)
    : BaseType(std::forward<Lambda>(lambda)) {
    using RtnType = std::invoke_result_t<Lambda, RenderNodeContext&>;
    static_assert(std::is_same_v<void, std::invoke_result_t<RtnType, RHIContext&>>);
  };

  using BaseType::operator();

protected:
};

class RenderPassExecutor: public std::function<void(RHIContext&)> {
  using BaseType = std::function<void(RHIContext&)>;
public:
  RenderPassExecutor() = default;

  template<typename Lambda, 
  typename = std::enable_if_t<!std::is_same_v<std::decay_t<Lambda>, RenderPassExecutor> && !std::is_void_v<Lambda>>>
  RenderPassExecutor(Lambda&& lambda)
    : BaseType(std::forward<Lambda>(lambda)) {
    static_assert(std::is_same_v<void, std::invoke_result_t<Lambda, RHIContext&>>);
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

  void connect(RenderNode& fromNode, std::string_view fromRes, RenderNode& toNode, std::string_view toRes);

  bool execute() const;

  bool compile();
  void setOutput(std::string_view passName, std::string_view resName);
  void setOutput(RenderNode& node, std::string_view resName);
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
  RHIResource::scptr_t mOutputResource;

private:
  void topologySortVisitor(std::vector<RenderNode*>& sortedNodes, RenderNode* node) const;
};

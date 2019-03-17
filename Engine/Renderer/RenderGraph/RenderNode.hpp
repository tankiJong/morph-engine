#pragma once
#include "Engine/Core/common.hpp"

#include <vector>
#include <unordered_set>
#include <atomic>

#include "Engine/Graphics/RHI/Fence.hpp"

#include "Engine/Renderer/RenderGraph/RenderEdge.hpp"
#include "Engine/Renderer/RenderGraph/RenderNodeContext.hpp"
#include "Engine/Renderer/RenderGraph/RenderNodeBuilder.hpp"

class RenderPass;
class RenderNode;
class RenderGraph;
class RenderGraphResourceSet;
class RenderNode {

  friend class RenderGraph;
public:
  RenderNode(RenderGraph* owner, std::string_view name, RenderPass* pass)
  : mOwner(owner), mPass(pass), mName(name), mNodeContext(this) {}
  ~RenderNode();

  const std::string& name() const { return mName; }
  void init();
  void build();
  void dependOn(RenderNode& dependency);
  void connectInput(const RenderEdge& edge);
  void connectOutput(const RenderEdge& edge);

  RenderEdge::BindingInfo* res(std::string_view name);

  void run(RHIContext& ctx);
  // struct Inst {
  //   Inst(const Inst&) = delete;
  //   const RenderNode* node = nullptr;
  //   std::atomic_size_t decrement;
  //
  //   void issue(RHIContext& ctx);;
  //
  //   void resolveBarriers(RHIContext& ctx) const;
  // };
  //
  // Inst instantiate() const;

  const RenderGraphResourceSet& resourceSet() const;
  RenderGraphResourceSet& resourceSet();

protected:

  RenderGraph* mOwner = nullptr;
  RenderPass* mPass = nullptr;
  std::string mName = "Unnamed";

  std::vector<const RenderEdge*> mInputs;
  std::vector<const RenderEdge*> mOutputs;
  std::unordered_set<RenderNode*> mOutGoingNodes;
  std::unordered_set<RenderNode*> mInComingNodes;
  RenderNodeContext mNodeContext { this };
  RenderNodeBuilder mNodeBuilder { this };

  bool mVisited = false;
private:
  bool mIsCompiled = false;
};
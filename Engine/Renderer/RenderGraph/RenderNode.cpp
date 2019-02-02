#include "RenderNode.hpp"
#include "Engine/Renderer/RenderGraph/RenderGraph.hpp"
#include "Engine/Renderer/RenderGraph/RenderPass.hpp"
#include "Engine/Debug/Log.hpp"

void RenderNode::init() {
  mPass->construct(mNodeContext);
}

void RenderNode::dependOn(RenderNode& dependency) {
  {
    auto [ _, success ] = dependency.mOutGoingNodes.insert(&dependency);
    if(!success) {
      Log::logf("fail to insert node dependency: `%s` -> `%s`, dependency already exists.", mName.c_str(), dependency.mName.c_str());
      DEBUGBREAK;
    }
  }
  {
    auto [ _, success ] = mInComingNodes.insert(&dependency);
    if(!success) {
      Log::logf("fail to insert node dependency: `%s` <- `%s`, dependency already exists.", dependency.mName.c_str(), mName.c_str());
      DEBUGBREAK;
    }
  }
}

RenderEdge::BindingInfo* RenderNode::res(std::string_view name) {
  return mNodeContext.find(name);
}

void RenderNode::Inst::resolveBarriers(RHIContext& ctx) const {
  
}

void RenderNode::Inst::issue(RHIContext& ctx) {
  resolveBarriers(ctx);
  node->mNodeContext.apply(ctx);
  node->mPass->execute(ctx);
}

RenderNode::Inst RenderNode::instantiate() const {
  return { this, mInComingNodes.size() + 1u, Fence::create() };

  // inst.fence->gpuVaule() = mInputs.size()

}

RenderNode::~RenderNode() {
  mOwner->freePass(mPass);  
}

void RenderNode::connectInput(const RenderEdge& edge) {
  EXPECTS(edge.to == this);
  EXPECTS(mNodeContext.exists(edge.toRes));
  mInputs.push_back(&edge);
}

void RenderNode::connectOutput(const RenderEdge& edge) {
  EXPECTS(edge.from == this);
  EXPECTS(mNodeContext.exists(edge.fromRes));
  mOutputs.push_back(&edge);
}

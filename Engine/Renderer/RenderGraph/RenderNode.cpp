#include "RenderNode.hpp"
#include "Engine/Renderer/RenderGraph/RenderGraph.hpp"
#include "Engine/Renderer/RenderGraph/RenderPass.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

void RenderNode::init() {
  mPass->construct(mNodeBuilder, mNodeContext);
}

void RenderNode::build() {
  if(mIsCompiled) return;

  // resolve hidden dependency from input
  Log::log("============ Ignore dependency warnings begin ===========");
  for(const RenderEdge* input: mInputs) {
    if(input->from != nullptr) {
      dependOn(*input->from);
    }
  }
  Log::log("============ Ignore dependency warnings end =============");

  mNodeContext.build();

  // later, also need to resolve the scope for all transient resources

  mIsCompiled = true;
}

void RenderNode::dependOn(RenderNode& dependency) {
  {
    auto [ _, success ] = dependency.mOutGoingNodes.insert(this);
    if(!success) {
      Log::warnf("fail to insert node dependency: `%s` -> `%s`, dependency already exists.", mName.c_str(), dependency.mName.c_str());
      // DEBUGBREAK;
    }
  }
  {
    auto [ _, success ] = mInComingNodes.insert(&dependency);
    if(!success) {
      Log::warnf("fail to insert node dependency: `%s` <- `%s`, dependency already exists.", dependency.mName.c_str(), mName.c_str());
      // DEBUGBREAK;
    }
  }
}

RenderEdge::BindingInfo* RenderNode::res(std::string_view name) {
  std::string fullname = mName + ".";
  fullname.append(name);

  return mNodeContext.find(fullname);
}

void RenderNode::run(RHIContext& ctx) {
  // resolve barrier
  SCOPED_GPU_EVENT(ctx, mName.c_str());

  RenderGraphResourceSet& set = resourceSet();

  for(const RenderEdge* input: mInputs) {
    
    RHIResource::sptr_t res = set.get<RHIResource>(*input->toRes->handle);
    ctx.transitionBarrier(res.get(), input->toRes->state);
  }

  for(const RenderEdge* output: mOutputs) {
    RHIResource::sptr_t res = set.get<RHIResource>(*output->fromRes->handle);
    ctx.transitionBarrier(res.get(), output->fromRes->state);
  }

  mNodeContext.apply(ctx);
  mPass->execute(set, ctx);
}

RenderGraphResourceSet& RenderNode::resourceSet() {
  return mOwner->mResourceSet;
}

//
// void RenderNode::Inst::resolveBarriers(RHIContext& ctx) const {
//   
// }
//
// void RenderNode::Inst::issue(RHIContext& ctx) {
//   resolveBarriers(ctx);
//   node->mNodeContext.apply(ctx);
//   node->mPass->execute(ctx);
// }
//
// RenderNode::Inst RenderNode::instantiate() const {
//
//   size_t totalDependency = 0;
//   totalDependency += mInComingNodes.size();
//
//   return { this,  totalDependency };
//
// }

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

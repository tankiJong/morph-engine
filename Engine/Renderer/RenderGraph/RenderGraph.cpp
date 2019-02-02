#include "RenderGraph.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Renderer/RenderGraph/RenderPass.hpp"
#include "Engine/Renderer/RenderGraph/RenderNode.hpp"

class FunctionalRenderPass: public RenderPass {

public:
  // ~FunctionalRenderPass() override;
  FunctionalRenderPass(RenderPassConstructor constructor)
    : mConstructor(std::move(constructor)) {}

  void construct(RenderNodeContext& builder) override {
    mExecutor = mConstructor(builder);
  };
  void execute(RHIContext& ctx) const override {
    mExecutor(ctx);
  };

protected:
  RenderPassExecutor mExecutor;
  RenderPassConstructor mConstructor;

};

RenderNode& RenderGraph::defineNode(std::string_view name, RenderPassConstructor constructor) {

  RenderPass* pass = allocatePass<FunctionalRenderPass>(constructor);
  return addPass(name, pass);
}

void RenderGraph::depend(RenderNode& blocker, RenderNode& blockee) {
  blockee.dependOn(blocker);
}

void RenderGraph::connect(RenderNode& fromNode, std::string_view fromRes, RenderNode& toNode, std::string_view toRes) {

  RenderEdge::BindingInfo* from = fromNode.res(fromRes);
  RenderEdge::BindingInfo* to = toNode.res(toRes);

  ENSURES(from != nullptr);
  ENSURES(to != nullptr);

  RenderEdge e { &fromNode, &toNode, from, to };
  auto [ edge, success ] = mEdges.insert(e);
  if(!success) {
    Log::logf("fail to insert resource dependency: `%s`-`%s` -> `%s`-`%s`, dependency already exists.", 
              fromNode.mName.c_str(), fromRes.data(), toNode.mName.c_str(), toRes.data());
    DEBUGBREAK;
  }
  fromNode.connectOutput(*edge);
  toNode.connectInput(*edge);
}

bool RenderGraph::execute() const {
  EXPECTS(mOutputNode != nullptr && mOutputResource != nullptr);
  
  return false;
}


void RenderGraph::setOutput(std::string_view passName, std::string_view resName) {
  
}

void RenderGraph::setOutput(RenderNode& node, std::string_view resName) {
  EXPECTS(node.mOwner == this);
  mOutputNode = &node;
  auto* bindingInfo = node.res(resName);
  mOutputResource = bindingInfo->res;
}

RenderNode& RenderGraph::addPass(std::string_view name, RenderPass* pass) {
  EXPECTS(pass != nullptr);

  if(auto kv = mDefinedNodes.find(name); kv != mDefinedNodes.end()) {
    Log::logf("Node `%s` already exists in the Render Graph", name.data());
  }

  RenderNode* node = new RenderNode{this, name, pass};
  node->init();

  mDefinedNodes[std::string(name)] = node;
}

void RenderGraph::freePass(RenderPass* pass) {
  delete pass;
}

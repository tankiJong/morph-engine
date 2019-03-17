#include "RenderNodeContext.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Graphics/Program/ProgramInst.hpp"
#include "Engine/Graphics/RHI/VertexLayout.hpp"
#include "Engine/Graphics/Model/Vertex.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Renderer/RenderGraph/RenderNode.hpp"
#include "Engine/Renderer/RenderGraph/RenderGraphResourceSet.hpp"

RenderNodeContext::~RenderNodeContext() {
}


// return true if is param
inline bool isParamOrExternal(std::string_view name) {
  return name[0] == '.';
}

void RenderNodeContext::readSrv(std::string_view name, uint registerIndex, uint registerSpace) {
  bool isParam = isParamOrExternal(name);

  if(isParam) {
    addParamBindingInfo(name, 
                        mForCompute 
                        ? RHIResource::State::NonPixelShader
                        : RHIResource::State::ShaderResource,
                        registerIndex, registerSpace);
  } else {
    addExternalBindingInfo(name, 
                        mForCompute 
                        ? RHIResource::State::NonPixelShader
                        : RHIResource::State::ShaderResource,
                        registerIndex, registerSpace);
  }

}

void RenderNodeContext::readCbv(std::string_view name, uint registerIndex, uint registerSpace) {
  bool isParam = isParamOrExternal(name);

  if(isParam) {
    addParamBindingInfo(name,
                 RHIResource::State::ConstantBuffer,
                      registerIndex, registerSpace);
  } else {
    addExternalBindingInfo(name,
                 RHIResource::State::ConstantBuffer,
                      registerIndex, registerSpace);
  }

}

void RenderNodeContext::writeRtv(std::string_view name, uint registerIndex) {
  bool isParam = isParamOrExternal(name);

  if(isParam) {
    addParamBindingInfo(name,
                 RHIResource::State::RenderTarget,
                      registerIndex, 0);
  } else {
    addExternalBindingInfo(name,
                 RHIResource::State::RenderTarget,
                      registerIndex, 0);
  }

}

void RenderNodeContext::writeDsv(std::string_view name) {
  bool isParam = isParamOrExternal(name);

  if(isParam) {
    addParamBindingInfo(name,
                 RHIResource::State::DepthStencil,
                 0, 0);
  } else {
    addExternalBindingInfo(name,
                 RHIResource::State::DepthStencil,
                 0, 0);
  }

}

void RenderNodeContext::readWriteUav(std::string_view name, uint registerIndex, uint registerSpace) {
  bool isParam = isParamOrExternal(name);
  
  if(isParam) {
    addParamBindingInfo(name,
                 RHIResource::State::UnorderedAccess,
                 registerIndex, registerSpace);
  } else {
    addExternalBindingInfo(name,
                 RHIResource::State::UnorderedAccess,
                 registerIndex, registerSpace);
  }
  
}

void RenderNodeContext::readWriteUav(const RenderResourceHandle& res, uint registerIndex, uint registerSpace) {
  addBindingInfo(res, RHIResource::State::UnorderedAccess, registerIndex, registerSpace);
}

void RenderNodeContext::writeRtv(const RenderResourceHandle& res, uint registerIndex) {
  addBindingInfo(res, RHIResource::State::RenderTarget, registerIndex, 0);
}

void RenderNodeContext::reset(Program::scptr_t prog, bool forCompute) {
  mPiplineState = std::monostate();
  if(!mForCompute) {
    mFrameBuffer = FrameBuffer();
  }

  ProgramInst* inst = nullptr;

  if(forCompute) {
    inst = new ComputeProgramInst(prog);
  } else {
    inst = new GraphicsProgramInst(prog);
  }

  mForCompute = forCompute;
  mTargetProgram.reset(inst);
  mBindingInfos.clear();
}

RenderEdge::BindingInfo* RenderNodeContext::find(std::string_view name) {
  if(auto kv = mBindingInfos.find(name); kv != mBindingInfos.end()) {
    return &(kv->second);
  }
  Log::logf("Fail to find resource `%s`", name);
  DEBUGBREAK;
  return nullptr;
}

bool RenderNodeContext::exists(RenderEdge::BindingInfo* info) const {
  for(const auto& [k, v]: mBindingInfos) {
    if(&v == info) return true;
  }

  return false;
}

void RenderNodeContext::build() {
  if(!mTargetProgram) return;
  if(mForCompute) {
    buildForCompute();
  } else {
    buildForGraphics();
  }
}

void RenderNodeContext::apply(RHIContext& ctx) { 
  if(!mTargetProgram) return;

  for(auto& [name, info]: mBindingInfos) {
    RHIResource::scptr_t res = mOwner->resourceSet().get<const RHIResource>(*info.handle);
    bindRuntimeResource(*res, info);
  }

  if(mForCompute) {
    ctx.setComputeState(*std::get<ComputeState::sptr_t>(mPiplineState));
  } else {
    ctx.setGraphicsState(*std::get<GraphicsState::sptr_t>(mPiplineState));
    ctx.setFrameBuffer(mFrameBuffer);
  }
  mTargetProgram->apply(ctx, true);
}

RenderResourceHandle& RenderNodeContext::extend(const RenderResourceHandle& source, std::string name, const RenderGraphResourceDesc& desc) {

  name = mOwner->name() + "." + name;
  RenderResourceHandle& handle = resourceSet().extend(source, name, desc);
  return handle;
}

RenderGraphResourceSet& RenderNodeContext::resourceSet() const {
  return mOwner->resourceSet();
}

void RenderNodeContext::addParamBindingInfo(std::string_view param, RHIResource::State state, uint registerIndex, uint registerSpace) {

  std::string name = mOwner->name() + std::string(param);

  const RenderResourceHandle* handle
    = mOwner->resourceSet().find(name)->cast();


  addBindingInfo(*handle, state, registerIndex, registerSpace);
}

void RenderNodeContext::
  addExternalBindingInfo(std::string_view resName, RHIResource::State state, uint registerIndex, uint registerSpace) {

  const RenderResourceHandle* handle
    = mOwner->resourceSet().find(resName)->cast();


  addBindingInfo(*handle, state, registerIndex, registerSpace);
}

void RenderNodeContext::addBindingInfo(const RenderResourceHandle& resHandle, RHIResource::State state, uint registerIndex, uint registerSpace) {
  // 
  // switch(state) { 
  //   case RHIResource::State::ConstantBuffer:
  //     info.cbv = res->cbv();
  //   break;
  //   case RHIResource::State::RenderTarget:
  //     info.rtv = res->rtv();
  //   break;
  //   case RHIResource::State::UnorderedAccess:
  //     info.uav = res->uav();
  //   break;
  //   case RHIResource::State::DepthStencil:
  //     info.dsv = res->dsv();
  //   break;
  //   case RHIResource::State::ShaderResource: 
  //   case RHIResource::State::NonPixelShader: 
  //     info.srv = res->srv();
  //   break;
  //   
  //   case RHIResource::State::PreInitialized:
  //   case RHIResource::State::Undefined:
  //   case RHIResource::State::Common:
  //   case RHIResource::State::VertexBuffer:
  //   case RHIResource::State::IndexBuffer:
  //   case RHIResource::State::StreamOut:
  //   case RHIResource::State::IndirectArg:
  //   case RHIResource::State::CopyDest:
  //   case RHIResource::State::CopySource:
  //   case RHIResource::State::ResolveDest:
  //   case RHIResource::State::ResolveSource:
  //   case RHIResource::State::Present:
  //   case RHIResource::State::GenericRead:
  //   case RHIResource::State::Predication:
  //     BAD_CODE_PATH();
  //   break;
  // }

  
  RenderEdge::BindingInfo info;

  info.name = resHandle.name;
  info.state = state;
  info.regIndex = registerIndex;
  info.regSpace = registerSpace;

  info.handle = &resHandle;

  if(mBindingInfos.find(info.name) != mBindingInfos.end()) {
    DEBUGBREAK;
    Log::logf("the resource already exists, overriding...", info.name.data());
  }

  mBindingInfos[info.name] = info;
}

void RenderNodeContext::buildForCompute() {
  ComputeState::Desc desc;

  desc.setProgram(mTargetProgram->prog());
  desc.setRootSignature(mTargetProgram->prog()->rootSignature());

  auto computeState = ComputeState::create(desc);
  mPiplineState = computeState;
  computeState->handle()->SetName(make_wstring(mOwner->name()).c_str());
}

void RenderNodeContext::buildForGraphics() {
  GraphicsState::Desc desc;

  desc.setProgram(mTargetProgram->prog());
  desc.setRootSignature(mTargetProgram->prog()->rootSignature());

  // TODO: this is hard coded here, I actually need bind Vbo/Mesh here
  desc.setPrimTye(GraphicsState::PrimitiveType::Triangle);
  desc.setVertexLayout(VertexLayout::For<vertex_lit_t>());

  FrameBuffer::Desc fdesc;
  for(auto& [name, info]: mBindingInfos) {
    if(info.state == RHIResource::State::RenderTarget) {
      GUARANTEE_OR_DIE(info.handle->desc.texture2.format.has_value(), 
                       Stringf("Expect the resource `%s` has well predefined format to bind as rtv", info.handle->name.c_str()).c_str());
      // TODO: Making too specific assumption here
      fdesc.defineColorTarget(
        info.regIndex, info.handle->desc.texture2.format.value());
    }
    if(info.state == RHIResource::State::DepthStencil) {
      GUARANTEE_OR_DIE(info.handle->desc.texture2.format.has_value(), 
                 Stringf("Expect the resource `%s` has well predefined format to bind as dsv", info.handle->name.c_str()).c_str());
      fdesc.defineDepthTarget(info.handle->desc.texture2.format.value());
    }
  }
  desc.setFboDesc(fdesc);

  auto graphicsState = GraphicsState::create(desc);
  mPiplineState = graphicsState;
  graphicsState->handle()->SetName(make_wstring(mOwner->name()).c_str());
  
}

std::string RenderNodeContext::resolveResourceName(std::string name) const {
  return mOwner->name() + "." + name;
}


void RenderNodeContext::bindRuntimeResource(const RHIResource& res, RenderEdge::BindingInfo& info) {
  switch(info.state) { 
    case RHIResource::State::ConstantBuffer:
      mTargetProgram->setCbv(*res.cbv(), info.regIndex, info.regSpace);
    break;
    case RHIResource::State::UnorderedAccess:
      mTargetProgram->setUav(*res.uav(), info.regIndex, info.regSpace);
    break;
    case RHIResource::State::ShaderResource:
    case RHIResource::State::NonPixelShader:
      mTargetProgram->setSrv(*res.srv(), info.regIndex, info.regSpace);
    break;

    case RHIResource::State::RenderTarget:
      mFrameBuffer.setColorTarget(res.rtv(), info.regIndex);
    break;
    case RHIResource::State::DepthStencil:
      mFrameBuffer.setDepthStencilTarget(res.dsv());
    break;
    default:
      BAD_CODE_PATH();
  }
}

#include "Engine/Graphics/RHI/RHIBuffer.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "RHIContext.hpp"
#include "Engine/Core/Resource.hpp"
#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Graphics/Program/ProgramInst.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"

#include "Graphics/RHI/Blit_vs.h"
#include "Graphics/RHI/Blit_ps.h"
#include "Engine/Math/Primitives/AABB2.hpp"
#include "Engine/Graphics/RHI/FrameBuffer.hpp"
#include "Engine/Graphics/RHI/PipelineState.hpp"
#include "Engine/Graphics/RHI/VertexLayout.hpp"

void RHIContext::updateBuffer(RHIBuffer* buffer, const void* data, size_t offset, size_t byteCount) {
  if (byteCount == 0) {
    byteCount = buffer->size() - offset;
  }

  // falcor try to  tweek size and offset so that it's safe to do the operation
  // I will just assume the input is always legal first

  mCommandsPending = true;

  // Allocate a buffer on the upload heap

  byte* start = (byte*)data + offset;

  if(buffer->cpuAccess() == RHIBuffer::CPUAccess::Write) {
    buffer->updateData(data, offset, byteCount);
    return;
  }

  RHIBuffer::sptr_t uploadBuffer = RHIBuffer::create(byteCount, RHIBuffer::BindingFlag::None, RHIBuffer::CPUAccess::Write, start);

  copyBufferRegion(buffer, offset, uploadBuffer.get(), 0, byteCount);
}
//
// void RHIContext::updateTexture(RHITexture& texture, void* data) {
//   mCommandsPending = true;
//   // temp version, assume 2d, 1 mip
//   uint subresCount = 1;
//   
//   // if(3d) {}
//   // else {
//   updateTextureSubresources(texture, 0, subresCount, data);
// }


void RHIContext::blit(const ShaderResourceView& from, const RenderTargetView& to) {
  aabb2 fullscreen = {{0,0}, {1, 1}};
  blit(from, to, fullscreen, fullscreen);
}

void RHIContext::blit(
  const ShaderResourceView& from, const RenderTargetView& to, 
  const aabb2& fromRect, const aabb2& toRect) {

  struct BlitConstant {
    vec2 offset = vec2::zero;
    vec2 scale = vec2::one;
  };
  static auto prog = Resource<Program>::get("internal/shader/blit");

  auto inst = GraphicsProgramInst::create(prog);

  RHIResource::scptr_t destRes = to.res().lock();
  RHIResource::scptr_t srcRes = from.res().lock();
  RHITexture::scptr_t destTex = std::dynamic_pointer_cast<const RHITexture>(destRes);
  RHITexture::scptr_t srcTex = std::dynamic_pointer_cast<const RHITexture>(srcRes);
  ASSERT_OR_DIE(destTex != nullptr && srcTex != nullptr, "invalid dest resource for blit");

  uint destMipLevel = to.info().mostDetailedMip;

  {
    BlitConstant constants;
    constants.offset = fromRect.mins;
    constants.scale = fromRect.size();

    RHIBuffer::sptr_t blitCb = RHIBuffer::create(
      sizeof(BlitConstant), 
      RHIResource::BindingFlag::ConstantBuffer, 
      RHIBuffer::CPUAccess::None, &constants);
    inst->setCbv(*blitCb->cbv(), 0);
    transitionBarrier(blitCb.get(), RHIResource::State::ConstantBuffer);
  }

  aabb2 viewport;
  {
    vec2 size = vec2(destTex->size(destMipLevel));
   
    vec2 offset = size * toRect.mins;
    vec2 vsize = toRect.size() * size;

    viewport = { offset, vsize + offset };
  }

  inst->setSrv(from, 0);

  FrameBuffer::Desc fdesc;
  fdesc.defineColorTarget(0, destTex->format());
  FrameBuffer fbo(fdesc);
  fbo.setColorTarget(&to, 0);

  // auto tempTex = Texture2::create(destTex->width(destMipLevel), destTex->height(destMipLevel), destTex->format(), RHIResource::BindingFlag::RenderTarget);
  

  static GraphicsState::sptr_t graphicsState;
  if(graphicsState == nullptr) {
    GraphicsState::Desc desc;
    desc.setProgram(prog);
    desc.setFboDesc(fbo.desc());
    desc.setPrimTye(GraphicsState::PrimitiveType::Triangle);
    desc.setRootSignature(prog->rootSignature());
    graphicsState = GraphicsState::create(desc);
  }

  bindDescriptorHeap();
  setViewport(viewport);
  setScissorRect(viewport);
  transitionBarrier(srcRes.get(), RHIResource::State::ShaderResource, TRANSITION_FULL, &from.info());
  transitionBarrier(destTex.get(), RHIResource::State::RenderTarget, TRANSITION_FULL, &to.info());
  setGraphicsState(*graphicsState);
  inst->apply(*this, true);
  setPrimitiveTopology(DRAW_TRIANGES);
  setFrameBuffer(fbo);
  contextData()->commandList()->IASetIndexBuffer(nullptr);
  draw(0, 6);
  // copySubresource(*tempTex, 0, *destTex, to.info().mostDetailedMip);
  // flush();
}

DEF_RESOURCE(Program, "internal/shader/blit") {
  Program::sptr_t prog(new Program());

  prog->stage(SHADER_TYPE_VERTEX).setFromBinary(gBlit_vs, sizeof(gBlit_vs));
  prog->stage(SHADER_TYPE_FRAGMENT).setFromBinary(gBlit_ps, sizeof(gBlit_ps));

  prog->compile();

  return prog;
}
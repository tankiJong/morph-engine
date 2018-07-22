#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12Resource.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"
#include "Engine/Graphics/RHI/RHITexture.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Graphics/RHI/TypedBuffer.hpp"

RHIContext::sptr_t RHIContext::create(command_queue_handle_t commandQueue) {
  sptr_t ctx = sptr_t(new RHIContext());

  ctx->mContextData = RHIContextData::create(
    RHIContextData::CommandQueueType::Direct, commandQueue);

  ENSURES(ctx->mContextData != nullptr);

  return ctx;
}

// void RHIContext::initDefaultRenderTarget() {
//   D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
//     RHIDevice::get()->cpuDescriptorPool()->handle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->GetCPUDescriptorHandleForHeapStart());
//
//
//   uint bufferSize = RHIDevice::get()->nativeDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//   for (UINT n = 0; n < RHIDevice::FRAME_COUNT; n++) {
//     d3d_call(RHIDevice::get()->mSwapChain->GetBuffer(n, IID_PPV_ARGS(&rt[n])));
//     RHIDevice::get()->nativeDevice()->CreateRenderTargetView(rt[n], nullptr, rtvHandle);
//     rtvHandle.ptr += 1 * bufferSize;
//   }
//
//   uint frameIndex = RHIDevice::get()->mSwapChain->GetCurrentBackBufferIndex();
//
//   ZeroMemory(&rtBarrier, sizeof(rtBarrier));
//   D3D12_RESOURCE_BARRIER &barrier = rtBarrier;
//   rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//   rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // default to this
//   barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES; // default to this
//   barrier.Transition.pResource = rt[frameIndex];
// }

void RHIContext::flush() {
  if(mCommandsPending) {
    mContextData->flush();
    mCommandsPending = false;
  }
  mContextData->fence()->syncCpu();
}

void RHIContext::copyBufferRegion(const RHIBuffer* dst, size_t dstOffset, RHIBuffer* src, size_t srcOffset, size_t byteCount) {
  resourceBarrier(dst, RHIResource::State::CopyDest);
  resourceBarrier(src, RHIResource::State::CopySource);
  mContextData->commandList()->CopyBufferRegion(
    dst->handle(), dstOffset, 
    src->handle(), src->gpuAddressOffset() + srcOffset, 
    byteCount);
  mCommandsPending = true;
}

void RHIContext::resourceBarrier(const RHIResource* res, RHIResource::State newState) {
  // if resource has cpu access, no need to do anything

  const RHIBuffer* buffer = dynamic_cast<const RHIBuffer*>(res);
  if (buffer && buffer->cpuAccess() != RHIBuffer::CPUAccess::None)return;

  if(res->state() != newState) {
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = res->handle();
    barrier.Transition.StateBefore = asDx12ResourceState(res->state());
    barrier.Transition.StateAfter = asDx12ResourceState(newState);
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    mContextData->commandList()->ResourceBarrier(1, &barrier);
    mCommandsPending = true;

    res->mState = newState;
  }
}

void RHIContext::beforeFrame() {
  D3D12_VIEWPORT mViewport;
  D3D12_RECT mScissorRect;

  mViewport.TopLeftX = 0;
  mViewport.TopLeftY = 0;
  mViewport.Width = Window::Get()->bounds().width();
  mViewport.Height = Window::Get()->bounds().height();
  mViewport.MinDepth = D3D12_MIN_DEPTH;
  mViewport.MaxDepth = D3D12_MAX_DEPTH;

  mScissorRect.left = 0;
  mScissorRect.top = 0;
  mScissorRect.right = Window::Get()->bounds().width();
  mScissorRect.bottom = Window::Get()->bounds().height();

  mContextData->commandList()->RSSetViewports(1, &mViewport);
  mContextData->commandList()->RSSetScissorRects(1, &mScissorRect);

  // D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
  //   RHIDevice::get()->cpuDescriptorPool()->handle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->GetCPUDescriptorHandleForHeapStart());
  //
  // uint frameIndex = RHIDevice::get()->mSwapChain->GetCurrentBackBufferIndex();
  // rtBarrier.Transition.pResource = rt[frameIndex];
  // rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  // rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  // // Indicate that the back buffer will be used as a render target.
  // mContextData->commandList()->ResourceBarrier(1, &rtBarrier);
  //
  // uint size = RHIDevice::get()->nativeDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  // rtvHandle.ptr += frameIndex * size;
  //
  // mContextData->commandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
  //
  // const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  // mContextData->commandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void RHIContext::afterFrame() {
  // uint frameIndex = RHIDevice::get()->mSwapChain->GetCurrentBackBufferIndex();
  // rtBarrier.Transition.pResource = rt[frameIndex];
  // rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  // rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  // // Indicate that the back buffer will be used as a render target.
  // mContextData->commandList()->ResourceBarrier(1, &rtBarrier);
}

void RHIContext::draw(uint start, uint count) {
  drawInstanced(start, 0, count, 1);
}

void RHIContext::drawIndexed(uint vertStart, uint idxStart, uint count) {
  mContextData->commandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  mContextData->commandList()->DrawIndexedInstanced(count, 1, idxStart, vertStart, 0);
  mCommandsPending = true;
}

void RHIContext::drawInstanced(uint startVert, uint startIns, uint vertCount, uint insCount) {
  mContextData->commandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  mContextData->commandList()->DrawInstanced(vertCount, insCount, startVert, startIns);
  mCommandsPending = true;
}

void RHIContext::setPipelineState(const PipelineState::sptr_t& pso) {
  mContextData->commandList()->SetGraphicsRootSignature(pso->rootSignature()->handle());
  mContextData->commandList()->SetPipelineState(pso->handle());
}

void RHIContext::setFrameBuffer(const FrameBuffer& fbo) {

  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[FrameBuffer::NUM_MAX_COLOR_TARGET];
  for(int                     i = 0; i < FrameBuffer::NUM_MAX_COLOR_TARGET; i++) {
    if(fbo.colorTarget(i)) {
      rtvHandles[i] = fbo.colorTarget(i)->rtv().handle()->cpuHandle(0);
    } else {
      rtvHandles[i] = RenderTargetView::nullView()->handle()->cpuHandle(0);
    }
  }

  D3D12_CPU_DESCRIPTOR_HANDLE rtvDepthHandle;
  if(fbo.depthStencilTarget()) {
    rtvDepthHandle = fbo.depthStencilTarget()->dsv()->handle()->cpuHandle(0);

  } else {
    rtvDepthHandle = DepthStencilView::nullView()->handle()->cpuHandle(0);
  }
  mContextData->commandList()->OMSetRenderTargets(FrameBuffer::NUM_MAX_COLOR_TARGET, rtvHandles, false, &rtvDepthHandle);
  
  mCommandsPending = true;
}

void RHIContext::setVertexBuffer(const VertexBuffer::sptr_t& vbo, uint streamIndex) {
  D3D12_VERTEX_BUFFER_VIEW vb = {};

  vb.BufferLocation = vbo->res().gpuAddress();
  vb.StrideInBytes = vbo->stride();
  vb.SizeInBytes = vbo->res().size();

  mContextData->commandList()->IASetVertexBuffers(streamIndex, 1, &vb);
}

void RHIContext::setIndexBuffer(const IndexBuffer::sptr_t& ibo) {
  D3D12_INDEX_BUFFER_VIEW ib = {};

  ib.BufferLocation = ibo->res().gpuAddress();
  ib.SizeInBytes = ibo->res().size();
  ib.Format = DXGI_FORMAT_R32_UINT;
  mContextData->commandList()->IASetIndexBuffer(&ib);
}

void RHIContext::bindDescriptorHeap() {
  DescriptorPool::sptr_t pool = RHIDevice::get()->gpuDescriptorPool();
  DescriptorPoolRhiData* rhiData = pool->rhiData();

  constexpr uint numHeap = count_of(rhiData->heaps);
  ID3D12DescriptorHeap* heaps[numHeap];

  uint heapCount = 0;
  for(uint i = 0; i <count_of(rhiData->heaps); i++) {
    if(rhiData->heaps[i]) {
      heaps[heapCount++] = rhiData->heaps[i]->handle();
    }
  }

  mContextData->commandList()->SetDescriptorHeaps(heapCount, heaps);
}

void RHIContext::clearRenderTarget(const RenderTargetView& rtv, const Rgba& rgba) {
  S<RHIResource> ptr = rtv.res().lock();

  vec4 color = rgba.normalized();
  EXPECTS(ptr);

  resourceBarrier(ptr.get(), RHIResource::State::RenderTarget);
  mContextData->commandList()->ClearRenderTargetView(rtv.handle()->cpuHandle(0), (FLOAT*)&color, 0, nullptr);
}

void RHIContext::clearDepthStencilTarget(const DepthStencilView& dsv, bool clearDepth, bool clearStencil, float depth, u8 stencil) {
  
  uint flag = clearDepth ? D3D12_CLEAR_FLAG_DEPTH : 0;

  flag |= clearStencil ? D3D12_CLEAR_FLAG_STENCIL : 0;

  if (flag == 0) return;

  S<RHIResource> res = dsv.res().lock();
  resourceBarrier(res.get(), RHIResource::State::DepthStencil);

  mContextData->commandList()
    ->ClearDepthStencilView(
      dsv.handle()->cpuHandle(0), D3D12_CLEAR_FLAGS(flag), depth, stencil, 0, nullptr);

  mCommandsPending = true;
}


void RHIContext::updateTextureSubresources(const RHITexture& texture, uint firstSubresource, uint subresourceCount, const void* data) {
  // mCommandsPending = true;
  //
  // TODO("support different type of texture");
  // uint arraySize = 1;
  //
  UNIMPLEMENTED();

}

void RHIContext::updateTexture(const RHITexture& texture, const void* data) {
  mCommandsPending = true;

  u64 uploadBufferSize = GetRequiredIntermediateSize(texture.handle(), 0, 1);

  auto buffer = RHIBuffer::create(uploadBufferSize, RHIBuffer::BindingFlag::None, RHIBuffer::CPUAccess::Write, nullptr);


  ID3D12Resource* textureUploadHeap = buffer->handle();

  //
  resourceBarrier(&texture, RHIResource::State::CopyDest);

  uint pixelSize;

  switch(texture.format()) { 
    case TEXTURE_FORMAT_UNKNOWN:
      pixelSize = 0;
    break;
    case TEXTURE_FORMAT_RGBA8: 
      pixelSize = 4;
    break;
    case TEXTURE_FORMAT_RG8: 
      pixelSize = 2;
    break;
    case TEXTURE_FORMAT_R8:
      pixelSize = 1;
    break;
    case TEXTURE_FORMAT_RGBA16: 
      pixelSize = 8;
    break;
    case TEXTURE_FORMAT_RG16: 
      pixelSize = 4;
    break;
    case TEXTURE_FORMAT_R16: 
      pixelSize = 2;
    break;
    case TEXTURE_FORMAT_D24S8:
      pixelSize = 4;
    break;
    default:
      ERROR_AND_DIE("missing format");
  }
  D3D12_SUBRESOURCE_DATA textureData = {};
  textureData.pData = data;
  textureData.RowPitch = texture.width() * pixelSize;
  textureData.SlicePitch = textureData.RowPitch * texture.height();

  UpdateSubresources(mContextData->commandList(), texture.handle(), 
                     textureUploadHeap, 0, 0, 1, &textureData);

  // buffer->unmap();
  // flush();
}

template<typename T>
class foo {
  template<typename U>
  operator foo<U>() {
    return *this;
  }
};
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12Resource.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"
#include "Engine/Graphics/RHI/RHITexture.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Graphics/RHI/TypedBuffer.hpp"
#include "Engine/Graphics/RHI/RHIContextData.hpp"
#include "Engine/Graphics/RHI/DescriptorPool.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"
#include "Engine/Graphics/RHI/PipelineState.hpp"
#include "ThirdParty/WinPixEventRuntime/Include/pix3.h"
#include "Engine/Math/MathUtils.hpp"

#pragma comment(lib, "ThirdParty/WinPixEventRuntime/bin/WinPixEventRuntime.lib")


static void setTransitionBarrier(const RHIResource& res, RHIResource::State newState, RHIResource::State oldState, uint subresourceIndex, ID3D12GraphicsCommandList* cmdList, eTransitionBarrierFlag flag) {

  D3D12_RESOURCE_BARRIER barrier;

  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  switch(flag) { 
    case TRANSITION_FULL: 
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    break;
    case TRANSITION_BEGIN:
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
    break;
    case TRANSITION_END: 
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
    break;
  }

  barrier.Transition.pResource = res.handle().Get();
  barrier.Transition.StateBefore = asDx12ResourceState(oldState);
  barrier.Transition.StateAfter = asDx12ResourceState(newState);
  barrier.Transition.Subresource = subresourceIndex;

  cmdList->ResourceBarrier(1, &barrier);

}


static bool setGlobalState(const RHIResource& res, RHIResource::State state, ID3D12GraphicsCommandList* cmdList, eTransitionBarrierFlag flag) {
  if(res.globalState() == state) return false;

  setTransitionBarrier(
    res, state, res.globalState(), 
    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, cmdList, flag);
  return true;
}
RHIContext::sptr_t RHIContext::create(command_queue_handle_t commandQueue) {
  sptr_t ctx = sptr_t(new RHIContext());

  ctx->mContextData = RHIContextData::create(
    RHIContextData::CommandQueueType::Direct, commandQueue);

  ENSURES(ctx->mContextData != nullptr);

  return ctx;
}

void RHIContext::textureBarrier(const RHITexture& tex, RHIResource::State newState, eTransitionBarrierFlag flag) {

  bool success = setGlobalState(tex, newState, mContextData->commandList().Get(), flag);
  if(flag != TRANSITION_BEGIN) {
    tex.setGlobalState(newState);
  }
  tex.markGlobalInTransition(flag == TRANSITION_BEGIN);

  mCommandsPending = mCommandsPending || success;

}

void RHIContext::bufferBarrier(const RHIBuffer& buffer, RHIResource::State newState, eTransitionBarrierFlag flag) {

  if(buffer.cpuAccess() != RHIBuffer::CPUAccess::None) return;
  bool success = setGlobalState(buffer, newState, mContextData->commandList().Get(), flag);
  if(flag != TRANSITION_BEGIN) {
    buffer.setGlobalState(newState);
  }
  buffer.markGlobalInTransition(flag == TRANSITION_BEGIN);


  mCommandsPending = mCommandsPending || success;

}

void RHIContext::subresourceBarrier(const RHITexture& tex,
  RHIResource::State newState,
  const ResourceViewInfo* viewInfo,
  eTransitionBarrierFlag flag) {

  ResourceViewInfo fullResInfo;

  bool setGlobal = false;

  if(viewInfo == nullptr) {
    fullResInfo.arraySize = tex.arraySize();
    fullResInfo.firstArraySlice = 0;
    fullResInfo.mostDetailedMip = 0;
    fullResInfo.mipCount = tex.mipCount();
    viewInfo = &fullResInfo;
    setGlobal = true;
  }

  for(uint arraySlice = viewInfo->firstArraySlice; 
      arraySlice < viewInfo->firstArraySlice + viewInfo->arraySize; 
      arraySlice++) {
    for(uint mip = viewInfo->mostDetailedMip; 
        mip < viewInfo->mipCount + viewInfo->mostDetailedMip; 
        mip++) {
      
      RHIResource::State oldState = tex.subresourceState(arraySlice, mip);

      if(oldState != newState) {
        setTransitionBarrier(tex, newState, oldState, tex.subresourceIndex(arraySlice, mip),
                             mContextData->commandList().Get(), flag);
        if(!setGlobal) {
          if(flag != TRANSITION_BEGIN) {
            tex.setSubresourceState(arraySlice, mip, newState);
          }
          tex.markSubresourceInTransition(arraySlice, mip, flag == TRANSITION_BEGIN);
        }
        mCommandsPending = true;
      }
    }
  }

  if(setGlobal) tex.setGlobalState(newState);
  
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
  transitionBarrier(dst, RHIResource::State::CopyDest);
  transitionBarrier(src, RHIResource::State::CopySource);
  mContextData->commandList()->CopyBufferRegion(
    dst->handle().Get(), dstOffset, 
    src->handle().Get(), src->gpuAddressOffset() + srcOffset, 
    byteCount);
  mCommandsPending = true;
}

void RHIContext::transitionBarrier(const RHIResource* res, RHIResource::State newState, eTransitionBarrierFlag flags, const ResourceViewInfo* viewInfo) {
  // if resource has cpu access, no need to do anything
  const RHITexture* tex = dynamic_cast<const RHITexture*>(res);
  if(tex) {
    bool globalBarrier = tex->isStateGlobal();
    if(viewInfo) {
      globalBarrier = globalBarrier && (viewInfo->firstArraySlice == 0);
      globalBarrier = globalBarrier && (viewInfo->mostDetailedMip == 0);
      globalBarrier = globalBarrier && (viewInfo->mipCount == tex->mipCount());
      globalBarrier = globalBarrier && (viewInfo->arraySize == tex->arraySize());
    }

    if(globalBarrier) {
      textureBarrier(*tex, newState, flags);
    } else {
      subresourceBarrier(*tex, newState, viewInfo, flags);
    }

  } else {
    const RHIBuffer* buffer = dynamic_cast<const RHIBuffer*>(res);
    if(buffer) {
      bufferBarrier(*buffer, newState, flags);
    }
  }
}

void RHIContext::uavBarrier(const RHIResource* res) {
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource = res->handle().Get();
    
    mContextData->commandList()->ResourceBarrier(1, &barrier);
    mCommandsPending = true;
}

// void RHIContext::beforeFrame() {
//   // D3D12_VIEWPORT mViewport;
//
//   // mViewport.TopLeftX = 0;
//   // mViewport.TopLeftY = 0;
//   // mViewport.Width = Window::Get()->bounds().width();
//   // mViewport.Height = Window::Get()->bounds().height();
//   // mViewport.MinDepth = D3D12_MIN_DEPTH;
//   // mViewport.MaxDepth = D3D12_MAX_DEPTH;
//
//
//
//   // mContextData->commandList()->RSSetViewports(1, &mViewport);
//
//   // D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
//   //   RHIDevice::get()->cpuDescriptorPool()->handle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->GetCPUDescriptorHandleForHeapStart());
//   //
//   // uint frameIndex = RHIDevice::get()->mSwapChain->GetCurrentBackBufferIndex();
//   // rtBarrier.Transition.pResource = rt[frameIndex];
//   // rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
//   // rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
//   // // Indicate that the back buffer will be used as a render target.
//   // mContextData->commandList()->ResourceBarrier(1, &rtBarrier);
//   //
//   // uint size = RHIDevice::get()->nativeDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//   // rtvHandle.ptr += frameIndex * size;
//   //
//   // mContextData->commandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
//   //
//   // const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
//   // mContextData->commandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
// }

void RHIContext::dispatch(uint threadGroupX, uint threadGroupY, uint threadGroupZ) {
  mContextData->commandList()->Dispatch(threadGroupX, threadGroupY, threadGroupZ);
}

void RHIContext::draw(uint start, uint count) {
  drawInstanced(start, 0, count, 1);
}

void RHIContext::drawIndexed(uint vertStart, uint idxStart, uint count) {
  mContextData->commandList()->DrawIndexedInstanced(count, 1, idxStart, vertStart, 0);
  mCommandsPending = true;
}

void RHIContext::drawInstanced(uint startVert, uint startIns, uint vertCount, uint insCount) {
  mContextData->commandList()->DrawInstanced(vertCount, insCount, startVert, startIns);
  mCommandsPending = true;
}

void RHIContext::setGraphicsRootSignature(const RootSignature& rootSig) {
  mContextData->commandList()->SetGraphicsRootSignature(rootSig.handle().Get());
}

void RHIContext::setComputeRootSignature(const RootSignature& rootSig) {
  mContextData->commandList()->SetComputeRootSignature(rootSig.handle().Get());
}

void RHIContext::setGraphicsState(const GraphicsState& pso) {
  mContextData->commandList()->SetPipelineState(pso.handle().Get());
  if(pso.rootSignature()) {
    mContextData->commandList()->SetGraphicsRootSignature(pso.rootSignature()->handle().Get());
  }
}

void RHIContext::setComputeState(const ComputeState& pso) {
  if(pso.rootSignature()) {
    mContextData->commandList()->SetComputeRootSignature(pso.rootSignature()->handle().Get());
  }
  mContextData->commandList()->SetPipelineState(pso.handle().Get());
}

void RHIContext::setFrameBuffer(const FrameBuffer& fbo) {

  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[FrameBuffer::NUM_MAX_COLOR_TARGET];
  for(int i = 0; i < FrameBuffer::NUM_MAX_COLOR_TARGET; i++) {
    const RenderTargetView* rtv = fbo.colorTarget(i);
    if(rtv) {
      rtvHandles[i] = rtv->handle()->cpuHandle(0);
      RHIResource::scptr_t res = rtv->res().lock();
      transitionBarrier(res.get(), RHIResource::State::RenderTarget, TRANSITION_FULL, &rtv->info());
    } else {
      rtvHandles[i] = RenderTargetView::nullView()->handle()->cpuHandle(0);
    }
  }

  D3D12_CPU_DESCRIPTOR_HANDLE rtvDepthHandle;
  if(fbo.depthStencilTarget()) {
    rtvDepthHandle = fbo.depthStencilTarget()->handle()->cpuHandle(0);

    RHIResource::scptr_t res = fbo.depthStencilTarget()->res().lock();
    transitionBarrier(res.get(), RHIResource::State::DepthStencil);

  } else {
    rtvDepthHandle = DepthStencilView::nullView()->handle()->cpuHandle(0);
  }
  mContextData->commandList()->OMSetRenderTargets(FrameBuffer::NUM_MAX_COLOR_TARGET, rtvHandles, false, &rtvDepthHandle);
  
  mCommandsPending = true;
}

void RHIContext::setVertexBuffer(const VertexBuffer& vbo, uint streamIndex) {
  D3D12_VERTEX_BUFFER_VIEW vb = {};

  vb.BufferLocation = vbo.res().gpuAddress();
  vb.StrideInBytes  = (UINT)vbo.stride();
  vb.SizeInBytes    = (UINT)vbo.res().size();

  mContextData->commandList()->IASetVertexBuffers(streamIndex, 1, &vb);
}

void RHIContext::setIndexBuffer(const IndexBuffer* ibo) {
  if (ibo == nullptr) return;
  D3D12_INDEX_BUFFER_VIEW ib = {};

  ib.BufferLocation = ibo->res().gpuAddress();
  ib.SizeInBytes = (UINT)ibo->res().size();
  ib.Format = DXGI_FORMAT_R32_UINT;
  mContextData->commandList()->IASetIndexBuffer(&ib);
}

void RHIContext::setPrimitiveTopology(const eDrawPrimitive prim) {
  switch(prim) { 
    case DRAW_UNKNOWN:
      mContextData->commandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
    break;
    case DRAW_POINTS: 
      mContextData->commandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
    break;
    case DRAW_LINES: 
      mContextData->commandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    break;
    case DRAW_TRIANGES: 
      mContextData->commandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    break;
    case NUM_PRIMITIVE_TYPES:
    default: 
    BAD_CODE_PATH();
  }
}

void RHIContext::setViewport(const aabb2& bounds) {
  D3D12_VIEWPORT viewport;

  viewport.TopLeftX = bounds.mins.x;
  viewport.TopLeftY = bounds.mins.x;
  viewport.Width = bounds.width();
  viewport.Height = bounds.height();
  viewport.MinDepth = D3D12_MIN_DEPTH;
  viewport.MaxDepth = D3D12_MAX_DEPTH;


  mContextData->commandList()->RSSetViewports(1, &viewport);
}

void RHIContext::setScissorRect(const aabb2& rect) {
  D3D12_RECT scissorRect;

  scissorRect.left    = (LONG)rect.mins.x;
  scissorRect.top     = (LONG)rect.mins.y;
  scissorRect.right   = (LONG)rect.maxs.x;
  scissorRect.bottom  = (LONG)rect.maxs.y;
  mContextData->commandList()->RSSetScissorRects(1, &scissorRect);
}

void RHIContext::bindDescriptorHeap() {
  DescriptorPool::sptr_t pool = RHIDevice::get()->gpuDescriptorPool();
  DescriptorPoolRhiData* rhiData = pool->rhiData();

  constexpr uint numHeap = count_of(rhiData->heaps);
  ID3D12DescriptorHeap* heaps[numHeap];

  uint heapCount = 0;
  for(uint i = 0; i <count_of(rhiData->heaps); i++) {
    if(rhiData->heaps[i]) {
      heaps[heapCount++] = rhiData->heaps[i]->handle().Get();
    }
  }

  mContextData->commandList()->SetDescriptorHeaps(heapCount, heaps);
}

void RHIContext::clearRenderTarget(const RenderTargetView& rtv, const Rgba& rgba) {
  clearRenderTarget(rtv, rgba.normalized());
}

void RHIContext::clearRenderTarget(const RenderTargetView& rtv, const vec4& color) {
  
  RHIResource::scptr_t ptr = rtv.res().lock();

  EXPECTS(ptr);

  transitionBarrier(ptr.get(), RHIResource::State::RenderTarget);
  mContextData->commandList()->ClearRenderTargetView(rtv.handle()->cpuHandle(0), (FLOAT*)&color, 0, nullptr);
}

void RHIContext::clearDepthStencilTarget(const DepthStencilView& dsv, bool clearDepth, bool clearStencil, float depth, u8 stencil) {
  
  uint flag = clearDepth ? D3D12_CLEAR_FLAG_DEPTH : 0;

  flag |= clearStencil ? D3D12_CLEAR_FLAG_STENCIL : 0;

  if (flag == 0) return;

  RHIResource::scptr_t res = dsv.res().lock();
  transitionBarrier(res.get(), RHIResource::State::DepthStencil);

  mContextData->commandList()
    ->ClearDepthStencilView(
      dsv.handle()->cpuHandle(0), D3D12_CLEAR_FLAGS(flag), depth, stencil, 0, nullptr);

  mCommandsPending = true;
}

//https://blogs.msdn.microsoft.com/pix/winpixeventruntime/
void RHIContext::beginEvent(const char* name) {
  PIXBeginEvent(mContextData->commandList().Get(), PIX_COLOR_DEFAULT, name);
}

void RHIContext::endEvent() {
  PIXEndEvent(mContextData->commandList().Get());
}

void RHIContext::updateTextureSubresources(const RHITexture& texture, uint firstSubresource, uint subresourceCount, const void* data) {
  // mCommandsPending = true;
  //
  // TODO("support different type of texture");
  // uint arraySize = 1;
  //
  UNIMPLEMENTED();
  updateTextureSubresources(texture, firstSubresource, subresourceCount, data);
}

void RHIContext::updateTexture(const RHITexture& texture, const void* data) {
  mCommandsPending = true;

  u64 uploadBufferSize = GetRequiredIntermediateSize(texture.handle().Get(), 0, 1);

  auto buffer = RHIBuffer::create(uploadBufferSize, RHIBuffer::BindingFlag::None, RHIBuffer::CPUAccess::Write, nullptr);


  ID3D12Resource* textureUploadHeap = buffer->handle().Get();

  //
  transitionBarrier(&texture, RHIResource::State::CopyDest);

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

  UpdateSubresources(mContextData->commandList().Get(), texture.handle().Get(), 
                      textureUploadHeap, 0, 0, 1, &textureData);

  

  // buffer->unmap();
  // flush();
}

void RHIContext::copyResource(const RHIResource& from, RHIResource& to) {
  transitionBarrier(&from, RHIResource::State::CopySource);
  transitionBarrier(&to, RHIResource::State::CopyDest);

  mContextData->commandList()->CopyResource(to.handle().Get(), from.handle().Get());
  mCommandsPending = true;
}

void RHIContext::copySubresource(const RHITexture& from, uint fromSubIndex, const RHITexture& to, uint toSubIndex) {
  transitionBarrier(&from, RHIResource::State::CopySource);
  transitionBarrier(&to, RHIResource::State::CopyDest);

  D3D12_TEXTURE_COPY_LOCATION fromLoc;
  D3D12_TEXTURE_COPY_LOCATION toLoc;

  fromLoc.pResource = from.handle().Get();
  fromLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  fromLoc.SubresourceIndex = fromSubIndex;

  toLoc.pResource = to.handle().Get();
  toLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  toLoc.SubresourceIndex = toSubIndex;

  mContextData->commandList()->CopyTextureRegion(&toLoc, 0, 0, 0, &fromLoc, NULL);
  mCommandsPending = true;

}

size_t RHIContext::readBuffer(const RHIBuffer& res, void* data, size_t maxSize) {
  mCommandsPending = true;

  u64 readbackBufferSize = GetRequiredIntermediateSize(res.handle().Get(), 0, 1);

  auto buffer = RHIBuffer::create(readbackBufferSize, RHIBuffer::BindingFlag::None, RHIBuffer::CPUAccess::Read, nullptr);

  transitionBarrier(&res, RHIResource::State::CopySource);

  copyResource(res, *buffer);

  flush();

  void* buf = buffer->map(RHIBuffer::MapType::Read);

  size_t read = clamp<size_t>(res.size(), 0u, maxSize);
  memcpy(data, buf, read);

  buffer->unmap();

  return read;
}

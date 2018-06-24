#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12Resource.hpp"
#include "dx12util.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"
#include "Engine/Graphics/RHI/RHITexture.hpp"

RHIContext::sptr_t RHIContext::create(command_queue_handle_t commandQueue) {
  sptr_t ctx = sptr_t(new RHIContext());

  ctx->mContextData = RHIContextData::create(
    RHIContextData::CommandQueueType::Direct, commandQueue);

  ENSURES(ctx->mContextData != nullptr);

  ctx->flush();
  return ctx;
}

ID3D12Resource* rt[RHIDevice::FRAME_COUNT];
D3D12_RESOURCE_BARRIER rtBarrier;

void RHIContext::initDefaultRenderTarget() {
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
    RHIDevice::get()->cpuDescriptorPool()->handle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->GetCPUDescriptorHandleForHeapStart());


  uint bufferSize = RHIDevice::get()->nativeDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  for (UINT n = 0; n < RHIDevice::FRAME_COUNT; n++) {
    d3d_call(RHIDevice::get()->mSwapChain->GetBuffer(n, IID_PPV_ARGS(&rt[n])));
    RHIDevice::get()->nativeDevice()->CreateRenderTargetView(rt[n], nullptr, rtvHandle);
    rtvHandle.ptr += 1 * bufferSize;
  }

  uint frameIndex = RHIDevice::get()->mSwapChain->GetCurrentBackBufferIndex();

  ZeroMemory(&rtBarrier, sizeof(rtBarrier));
  D3D12_RESOURCE_BARRIER &barrier = rtBarrier;
  rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // default to this
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES; // default to this
  barrier.Transition.pResource = rt[frameIndex];
}

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
  mViewport.Width = 1280;
  mViewport.Height = 720;
  mViewport.MinDepth = D3D12_MIN_DEPTH;
  mViewport.MaxDepth = D3D12_MAX_DEPTH;

  mScissorRect.left = 0;
  mScissorRect.top = 0;
  mScissorRect.right = 1280;
  mScissorRect.bottom = 720;

  mContextData->commandList()->RSSetViewports(1, &mViewport);
  mContextData->commandList()->RSSetScissorRects(1, &mScissorRect);

  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
    RHIDevice::get()->cpuDescriptorPool()->handle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)->GetCPUDescriptorHandleForHeapStart());

  uint frameIndex = RHIDevice::get()->mSwapChain->GetCurrentBackBufferIndex();
  rtBarrier.Transition.pResource = rt[frameIndex];
  rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  // Indicate that the back buffer will be used as a render target.
  mContextData->commandList()->ResourceBarrier(1, &rtBarrier);

  uint size = RHIDevice::get()->nativeDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  rtvHandle.ptr += frameIndex * size;

  mContextData->commandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  mContextData->commandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void RHIContext::afterFrame() {
  uint frameIndex = RHIDevice::get()->mSwapChain->GetCurrentBackBufferIndex();
  rtBarrier.Transition.pResource = rt[frameIndex];
  rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  // Indicate that the back buffer will be used as a render target.
  mContextData->commandList()->ResourceBarrier(1, &rtBarrier);
  flush();
}

void RHIContext::draw(uint start, uint count) {
  drawInstanced(start, 0, count, 1);
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

void RHIContext::setVertexBuffer(const S<RHIBuffer>& vbo, uint elementSize, uint streamIndex) {
  D3D12_VERTEX_BUFFER_VIEW vb = {};

  vb.BufferLocation = vbo->gpuAddress();
  vb.StrideInBytes = elementSize;
  vb.SizeInBytes = vbo->size();

  mContextData->commandList()->IASetVertexBuffers(streamIndex, 1, &vb);
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

  // this need a new home, possibly in shader
  mContextData->commandList()->SetGraphicsRootDescriptorTable(0, RHIDevice::get()->gpuDescriptorPool()->rhiData()->heaps[D3D12_DESCRIPTOR_RANGE_TYPE_SRV]->gpuHandleBase());
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

  auto buffer = RHIBuffer::create(uploadBufferSize, RHIBuffer::BindingFlag::None, RHIBuffer::CPUAccess::Write, data);

  buffer->map(RHIBuffer::MapType::WriteDiscard);
  ID3D12Resource* textureUploadHeap = buffer->handle();

  //
  // D3D12_RESOURCE_DESC textureResourceDesc;
  // textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  // textureResourceDesc.Alignment = 0;
  // textureResourceDesc.Width = uploadBufferSize;
  // textureResourceDesc.Height = 1;
  // textureResourceDesc.DepthOrArraySize = 1;
  // textureResourceDesc.MipLevels = 1;
  // textureResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
  // textureResourceDesc.SampleDesc.Count = 1;
  // textureResourceDesc.SampleDesc.Quality = 0;
  // textureResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  // textureResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
  //
  // RHIDevice::get()->nativeDevice()->CreateCommittedResource(
  //   &DefaultHeapProps, 
  //   D3D12_HEAP_FLAG_NONE, 
  //   &textureResourceDesc, 
  //   D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap));
  resourceBarrier(&texture, RHIResource::State::CopyDest);

  D3D12_SUBRESOURCE_DATA textureData = {};
  textureData.pData = data;
  textureData.RowPitch = texture.width() * texture.height();
  textureData.SlicePitch = textureData.RowPitch * texture.height();

  UpdateSubresources(mContextData->commandList(), texture.handle(), 
                     textureUploadHeap, 0, 0, 1, &textureData);

  buffer->unmap();
  flush();
}

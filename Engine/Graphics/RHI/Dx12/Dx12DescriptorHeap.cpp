#include "Dx12DescriptorHeap.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

Dx12DescriptorHeap::sptr_t Dx12DescriptorHeap::create(D3D12_DESCRIPTOR_HEAP_TYPE type, uint descCount, bool shadervisible) {
  RHIDevice::rhi_handle_t device = RHIDevice::get()->nativeDevice();

  uint chunkCount = (descCount + DESC_PER_CHUNK - 1) / DESC_PER_CHUNK;

  sptr_t heap = sptr_t(new Dx12DescriptorHeap(type, chunkCount));

  D3D12_DESCRIPTOR_HEAP_DESC desc = {};

  desc.Flags = shadervisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  desc.Type = type;
  desc.NumDescriptors = chunkCount * DESC_PER_CHUNK;

  d3d_call(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap->mRhiHandle)));

  heap->mCpuHeapStart = heap->mRhiHandle->GetCPUDescriptorHandleForHeapStart();
  heap->mGpuHeapStart = heap->mRhiHandle->GetGPUDescriptorHandleForHeapStart();

  return heap;
}

Dx12DescriptorHeap::Dx12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t chunkCount)
: mChunckCount(chunkCount), mType(type){}

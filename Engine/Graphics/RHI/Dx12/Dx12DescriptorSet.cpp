#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"

D3D12_DESCRIPTOR_HEAP_TYPE asoDx12DescriptorHeapType(DescriptorPool::Type t);

Dx12DescriptorHeap* getHeap(const DescriptorPool& pool, DescriptorSet::Type type) {
  auto dxType = asoDx12DescriptorHeapType(type);
  Dx12DescriptorHeap* heap = pool.rhiData()->heaps[dxType].get();
  ENSURES(heap);
  ENSURES(heap->type() == dxType);
  return heap;
}

void setCpuHandle(DescriptorSet& set, uint rangeIndex, uint descIndex, const DescriptorSet::cpu_handle_t& handle) {
  auto dstHandle = set.cpuHandle(rangeIndex, descIndex);
  RHIDevice::get()->nativeDevice()->CopyDescriptorsSimple(1, dstHandle, handle, asoDx12DescriptorHeapType(set.range(rangeIndex).type));
}


bool DescriptorSet::rhiInit() {
  mRhiData = std::make_shared<DescriptorSetRhiData>();

  uint count = 0;
  auto type = mLayout.range(0).type;
  D3D12_DESCRIPTOR_HEAP_TYPE heapType = asoDx12DescriptorHeapType(type);
  UNUSED(heapType);
  mRhiData->rangeBaseOffset.resize(mLayout.rangeCount());

  for(size_t i = 0; i<mLayout.rangeCount(); i++) {
    const auto& range = mLayout.range(i);
    mRhiData->rangeBaseOffset[i] = count;
    EXPECTS(heapType == asoDx12DescriptorHeapType(range.type));
    count += range.descCount;

  }
  Dx12DescriptorHeap* heap = getHeap(*mPool, type);

  mRhiData->alloc = heap->allocateDescriptors(count);

  if(mRhiData->alloc == nullptr) {
    // defer release and try again
    mPool->executeDeferredRelease();
    mRhiData->alloc = heap->allocateDescriptors(count);
  }

  return mRhiData->alloc != nullptr;
}

heap_cpu_handle_t DescriptorSet::cpuHandle(uint rangeIndex, uint offset) {
  uint index = mRhiData->rangeBaseOffset[rangeIndex] + offset;
  auto rtn = mRhiData->alloc->cpuHandle(index);
  return rtn;
}

heap_gpu_handle_t DescriptorSet::gpuHandle(uint rangeIndex, uint offset) {
  uint index = mRhiData->rangeBaseOffset[rangeIndex] + offset;
  return mRhiData->alloc->gpuHandle(index);
}

void DescriptorSet::setCbv(uint rangeIndex, uint descIndex, const ConstantBufferView& view) {
  setCpuHandle(*this, rangeIndex, descIndex, view.handle()->cpuHandle(0));
}

void DescriptorSet::setSrv(uint rangeIndex, uint descIndex, const ShaderResourceView& view) {
  setCpuHandle(*this, rangeIndex, descIndex, view.handle()->cpuHandle(0));
}

void DescriptorSet::setUav(uint rangeIndex, uint descIndex, const UnorderedAccessView& view) {
  setCpuHandle(*this, rangeIndex, descIndex, view.handle()->cpuHandle(0));
}

void DescriptorSet::bindForGraphics(const RHIContext& ctx, const RootSignature& /*root*/, uint rootIndex) {
  auto commandlist = ctx.contextData()->commandList();
  auto handle = gpuHandle(0);
  commandlist->SetGraphicsRootDescriptorTable(rootIndex, handle);
}

void DescriptorSet::bindForCompute(const RHIContext& ctx, const RootSignature& /*root*/, uint rootIndex) {
  ctx.contextData()->commandList()->SetComputeRootDescriptorTable(rootIndex, gpuHandle(0));
}


void DescriptorSet::clear() {
  for(uint i = 0; i < mLayout.rangeCount(); i++) {
    auto& range = mLayout.range(i);
    switch(range.type) { 
      case Type::TextureSrv:
      case Type::TypedBufferSrv:
      case Type::StructuredBufferSrv:
      for(uint k = 0; k < range.descCount; k++) {
        setSrv(i, k, *ShaderResourceView::nullView());
      }
        break;
      case Type::TextureUav:
      case Type::TypedBufferUav:
      case Type::StructuredBufferUav:
        for (uint k = 0; k < range.descCount; k++) {
          setUav(i, k, *UnorderedAccessView::nullView());
        }
        break;
      case Type::Cbv:
        for (uint k = 0; k < range.descCount; k++) {
          setCbv(i, k, *ConstantBufferView::nullView());
        }
        break;
      default: 
        ERROR_AND_DIE("Should not be here");
    }
  }
}
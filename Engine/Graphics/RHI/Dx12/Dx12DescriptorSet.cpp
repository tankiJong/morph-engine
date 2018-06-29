#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"

D3D12_DESCRIPTOR_HEAP_TYPE asoDx12DescriptorHeapType(DescriptorPool::Type t);

Dx12DescriptorHeap* getHeap(const DescriptorPool& pool, DescriptorSet::Type type) {
  auto dxType = asoDx12DescriptorHeapType(type);
  Dx12DescriptorHeap* heap = pool.rhiData()->heaps[dxType].get();
  ENSURES(heap);
  ENSURES(heap->type() == dxType);
  return heap;
}

bool DescriptorSet::rhiInit() {
  mRhiData = std::make_shared<DescriptorSetRhiData>();

  uint count = 0;
  auto type = mLayout.range(0).type;
  D3D12_DESCRIPTOR_HEAP_TYPE heapType = asoDx12DescriptorHeapType(type);

  mRhiData->rangeBaseOffset.resize(mLayout.rangeCount());

  for(size_t i = 0; i<mLayout.rangeCount(); i++) {
    const auto& range = mLayout.range(i);
    mRhiData->rangeBaseOffset[i] = count;
    EXPECTS(heapType == asoDx12DescriptorHeapType(range.type));
    count += range.descCount;

    Dx12DescriptorHeap* heap = getHeap(*mPool, type);

    // mRhiData->alloc = heap->al
    UNIMPLEMENTED_RETURN(false);
  }

  return false;
}

#include "Engine/Graphics/RHI/DescriptorPool.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorHeap.hpp"

D3D12_DESCRIPTOR_HEAP_TYPE asoDx12DescriptorHeapType(DescriptorPool::Type t) {
  switch (t) {
    case DescriptorPool::Type::TextureSrv:
    case DescriptorPool::Type::TextureUav:
    case DescriptorPool::Type::StructuredBufferSrv:
    case DescriptorPool::Type::StructuredBufferUav:
    case DescriptorPool::Type::TypedBufferSrv:
    case DescriptorPool::Type::TypedBufferUav:
    case DescriptorPool::Type::Cbv:
      return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    case DescriptorPool::Type::Dsv:
      return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    case DescriptorPool::Type::Rtv:
      return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    case DescriptorPool::Type::Sampler:
      return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    default:
      ERROR_AND_DIE("should not reach here");
      return D3D12_DESCRIPTOR_HEAP_TYPE(-1);
  }
}

bool DescriptorPool::rhiInit() {
  static_assert((uint)Type::NUM_TYPE == 10);

  uint descCount[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {0};
  descCount[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]            = mDesc.mDescCount[(uint)Type::Rtv];
  descCount[D3D12_DESCRIPTOR_HEAP_TYPE_DSV]            = mDesc.mDescCount[(uint)Type::Dsv];
  descCount[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]        = mDesc.mDescCount[(uint32_t)Type::Sampler];
  descCount[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]    = mDesc.mDescCount[(uint32_t)Type::Cbv];
  descCount[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]   += mDesc.mDescCount[(uint32_t)Type::TextureUav]
                                                       + mDesc.mDescCount[(uint32_t)Type::TypedBufferUav]
                                                       + mDesc.mDescCount[(uint32_t)Type::StructuredBufferUav];
  descCount[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]   += mDesc.mDescCount[(uint32_t)Type::TextureSrv]
                                                       + mDesc.mDescCount[(uint32_t)Type::TypedBufferSrv]
                                                       + mDesc.mDescCount[(uint32_t)Type::StructuredBufferSrv];

  mData = std::make_shared<DescriptorPoolRhiData>();

  for(uint i = 0; i< count_of(mData->heaps); i++) {
    if(descCount[i]) {
      mData->heaps[i] = Dx12DescriptorHeap::create(D3D12_DESCRIPTOR_HEAP_TYPE(i), descCount[i], mDesc.mShaderVisible);
      if (!mData->heaps[i]) return false;
    }
  }

  return true;
}

DescriptorPool::rhi_handle_t DescriptorPool::handle(uint heapIndex) const {
  EXPECTS(heapIndex < count_of(mData->heaps));

  return mData->heaps[heapIndex]->handle();
}

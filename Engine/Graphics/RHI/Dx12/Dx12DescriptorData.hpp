#pragma once
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorHeap.hpp"
#include "dx12util.hpp"
#include <vector>

struct DescriptorPoolRhiData {
  Dx12DescriptorHeap::sptr_t heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};

struct DescriptorSetRhiData {
  Dx12DescriptorHeap::Allocation::sptr_t alloc;
  std::vector<uint32_t> rangeBaseOffset;
};
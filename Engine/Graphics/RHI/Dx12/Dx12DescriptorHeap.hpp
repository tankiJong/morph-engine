#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/Dx12/dx12util.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

class Dx12DescriptorHeap: public std::enable_shared_from_this<Dx12DescriptorHeap>{
  struct Chunk;
  class Allocation;
public:
  using sptr_t = S<Dx12DescriptorHeap>;
  using scptr_t = S<const Dx12DescriptorHeap>;
  using rhi_handle_t = descriptor_heap_handle_t;

  static sptr_t create(D3D12_DESCRIPTOR_HEAP_TYPE type, uint descCount, bool shadervisible = true);

  heap_cpu_handle_t cpuHandleBase() const { return mCpuHeapStart; }
  heap_gpu_handle_t gpuHandleBase() const { return mGpuHeapStart; }

  rhi_handle_t handle() const { return mRhiHandle; }
  D3D12_DESCRIPTOR_HEAP_TYPE type() const { return mType; }
  static const uint DESC_PER_CHUNK = 64;
public:
  class Allocation {
  public:
    using sptr_t = S<Allocation>;
    uint heapEntryIndex(uint index) const {
      EXPECTS(index < mDescCount);
      return index + mBaseIndex;
    }

    // Index is relative to the allocation
    heap_cpu_handle_t cpuHandle(uint index) const {
      return mHeap->cpuHandle(heapEntryIndex(index));
    };

    // Index is relative to the allocation
    heap_gpu_handle_t gpuHandle(uint index) const {
      return mHeap->gpuHandle(heapEntryIndex(index));
    }

  protected:
    friend class Dx12DescriptorHeap;
    static sptr_t create(Dx12DescriptorHeap::sptr_t heap, uint baseIndex, uint descCount, S<Chunk> chunk);
    Allocation(Dx12DescriptorHeap::sptr_t heap, uint baseIndex, uint descCount, S<Chunk> chunk): mHeap(heap), mBaseIndex(baseIndex), mDescCount(descCount), mChunk(chunk) {};
    uint mDescCount = 0;
    uint mBaseIndex = 0;
    Dx12DescriptorHeap::sptr_t mHeap;
    S<Chunk> mChunk;
  };
protected:
  Dx12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t chunkCount);
  heap_cpu_handle_t cpuHandle(uint index) const;
  heap_gpu_handle_t gpuHandle(uint index) const;
  heap_cpu_handle_t mCpuHeapStart;
  heap_gpu_handle_t mGpuHeapStart;
  rhi_handle_t mRhiHandle;
  uint mChunckCount;
  D3D12_DESCRIPTOR_HEAP_TYPE mType;
};

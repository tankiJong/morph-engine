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

bool Dx12DescriptorHeap::setupCurrentChunk(uint descCount) {
  if(mCurrentChunk) {
    if (mCurrentChunk->chunkCount * DESC_PER_CHUNK - mCurrentChunk->currentDesc >= descCount) return true;

    if(mCurrentChunk->allocCount == 0) {

      // current chunk is empty, and is big enough, so release and get a bigger one 
      if(mCurrentChunk->chunkCount * DESC_PER_CHUNK >= descCount) {
        mCurrentChunk->reset();
        return true;
      }
    }
  }

  // need a new one

  uint chunkCount = (descCount + DESC_PER_CHUNK - 1) / DESC_PER_CHUNK;
  if(chunkCount == 1 && !mFreeChunk.empty()) {
    mCurrentChunk = mFreeChunk.front();
    mFreeChunk.pop();
    mCurrentChunk->reset();
    return true;
  }

  // no left free chunk
  if(mAllocatedChunks + chunkCount > mChunkCount) {
    return false;
  }

  mCurrentChunk = Chunk::sptr_t(new Chunk(mAllocatedChunks, chunkCount));

  mAllocatedChunks += chunkCount;
  return true;
}

void Dx12DescriptorHeap::releaseChunk(Chunk::sptr_t chunk) {
  chunk->allocCount--;
  if(chunk->allocCount == 0 && chunk != mCurrentChunk) {
    mFreeChunk.push(chunk);
  }
}

Dx12DescriptorHeap::Allocation::~Allocation() {
  mHeap->releaseChunk(mChunk);
}

Dx12DescriptorHeap::Allocation::sptr_t Dx12DescriptorHeap::Allocation::create(Dx12DescriptorHeap::sptr_t heap,
  uint baseIndex,
  uint descCount,
  S<Chunk> chunk) {
  return sptr_t(new Allocation(heap, baseIndex, descCount, chunk));
}

Dx12DescriptorHeap::Allocation::sptr_t Dx12DescriptorHeap::allocateDescriptors(uint count) {
  if (!setupCurrentChunk(count)) return nullptr;

  if(mCurrentChunk->chunkCount * DESC_PER_CHUNK - mCurrentChunk->currentDesc < count) {
    return nullptr;
  }

  Allocation::sptr_t alloc = Allocation::create(shared_from_this(), mCurrentChunk->absoluteIndex(), count, mCurrentChunk);

  mCurrentChunk->allocCount++;
  mCurrentChunk->currentDesc += count;
  return alloc;
}

Dx12DescriptorHeap::Dx12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t chunkCount)
: mChunkCount(chunkCount), mType(type) {
  mDescriptorSize = RHIDevice::get()->nativeDevice()->GetDescriptorHandleIncrementSize(type);
}


heap_cpu_handle_t Dx12DescriptorHeap::cpuHandle(uint index) const {
  heap_cpu_handle_t base = mCpuHeapStart;
  base.ptr += mDescriptorSize * index;
  return base;
}

heap_gpu_handle_t Dx12DescriptorHeap::gpuHandle(uint index) const {
  heap_gpu_handle_t base = mGpuHeapStart;
  base.ptr += mDescriptorSize * index;
  return base;
}

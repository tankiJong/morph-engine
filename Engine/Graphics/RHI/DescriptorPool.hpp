#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHI.hpp"
#include <queue>

struct DescriptorPoolRhiData;
struct DescriptorSetRhiData;
class Fence;
class DescriptorPool: public std::enable_shared_from_this<DescriptorPool> {
public:
  using sptr_t = S<DescriptorPool>;
  using scptr_t = S<const DescriptorPool>;
  using cpu_handle_t = heap_cpu_handle_t;
  using gpu_handle_t = heap_gpu_handle_t;
  using rhi_handle_t = descriptor_heap_handle_t;
  enum class Type: uint {
    TextureSrv,
    TextureUav,
    TypedBufferSrv,
    TypedBufferUav,
    Cbv,
    StructuredBufferUav,
    StructuredBufferSrv,
    Dsv,
    Rtv,
    Sampler,

    NUM_TYPE,
  };
  class Desc {
  public:
    Desc& setDescCount(Type type, uint count) {
      uint t = (uint)type;
      mTotalDescCount -= mDescCount[t];
      mTotalDescCount += count;
      mDescCount[t] = count;
      return *this;
    };

    inline Desc& setShaderVisibility(bool visible) {
      mShaderVisible = visible; return *this;
    }
  protected:
    friend DescriptorPool;
    uint mDescCount[(uint)Type::NUM_TYPE] = { 0 };
    uint mTotalDescCount = 0;
    bool mShaderVisible = false;
  };

  rhi_handle_t handle(uint heapIndex) const;
  DescriptorPoolRhiData* rhiData() const { return mData.get(); }
  void executeDeferredRelease();
  static sptr_t create(const Desc& desc, S<Fence> fence);
protected:
  friend DescriptorSet;
  DescriptorPool(const Desc& desc, const S<Fence> fence);
  bool rhiInit();
  void releaseAllocation(S<DescriptorSetRhiData> alloc);

  struct alloc_release {
    S<DescriptorSetRhiData> data;
    u64 fenceVal;
    bool operator>(const alloc_release& rhs) const {
      return fenceVal > rhs.fenceVal;
    }
  };
  Desc mDesc;
  S<Fence> mFence;
  std::shared_ptr<DescriptorPoolRhiData> mData;
  std::priority_queue<alloc_release, std::vector<alloc_release>, std::greater<>> mDeferredReleases;
};

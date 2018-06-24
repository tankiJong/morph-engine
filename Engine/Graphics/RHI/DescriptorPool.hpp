#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/Fence.hpp"

struct DescriptorPoolRhiData;

class DescriptorPool: public std::enable_shared_from_this<DescriptorPool> {
public:
  using sptr_t = S<DescriptorPool>;
  using scptr_t = S<const DescriptorPool>;
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
  static sptr_t create(const Desc& desc, Fence::sptr_t fence);
protected:
  DescriptorPool(const Desc& desc, const Fence::sptr_t fence);
  bool rhiInit();
  Desc mDesc;
  Fence::sptr_t mFence;
  std::shared_ptr<DescriptorPoolRhiData> mData;
};

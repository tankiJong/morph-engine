#pragma once
#include "Engine/Core/common.hpp"

#include "RHI.hpp"

class RHIHeap {
public:
  enum  eUsage {
    USAGE_BUFFER,
    USAGE_TEXTURE_RT_DS,
    USAGE_TEXTURE_NON_RT_DS,
  };
  using sptr_t = S<RHIHeap>;

  gpu_memory_t handle() const { return mHandle; }

  static RHIHeap::sptr_t create(size_t minSize, heap_properties_t props, eUsage usage, size_t alignment = 0);
protected:
  gpu_memory_t mHandle;
  size_t mSize;
};

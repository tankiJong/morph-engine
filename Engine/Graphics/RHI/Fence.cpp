#include "RHI.hpp"
#include "Fence.hpp"

Fence::Fence(Fence&& mv) noexcept {
  mCpuValue = mv.mCpuValue;
  mHandle = mv.mHandle;
  mData = mv.mData;

  mv.mHandle = nullptr;
  mv.mCpuValue = 0;
  mv.mData = nullptr;
}
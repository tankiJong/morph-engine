#include "DescriptorPool.hpp"

void DescriptorPool::executeDeferredRelease() {
  u64 gpuVal = mFence->gpuVaule();
  while(!mDeferredReleases.empty() && mDeferredReleases.top().fenceVal <= gpuVal) {
    mDeferredReleases.pop();
  }
}

DescriptorPool::sptr_t DescriptorPool::create(const Desc& desc, Fence::sptr_t fence) {
  sptr_t pool = sptr_t(new DescriptorPool(desc, fence));
  return pool->rhiInit() ? pool : nullptr;
}

DescriptorPool::DescriptorPool(const Desc& desc, const Fence::sptr_t fence)
  : mDesc(desc), mFence(fence) {}


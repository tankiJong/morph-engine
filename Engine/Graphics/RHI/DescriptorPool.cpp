#include "DescriptorPool.hpp"

DescriptorPool::sptr_t DescriptorPool::create(const Desc& desc, Fence::sptr_t fence) {
  sptr_t pool = sptr_t(new DescriptorPool(desc, fence));
  return pool->rhiInit() ? pool : nullptr;
}

DescriptorPool::DescriptorPool(const Desc& desc, const Fence::sptr_t fence)
  : mDesc(desc), mFence(fence) {}


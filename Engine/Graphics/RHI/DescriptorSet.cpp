#include "DescriptorSet.hpp"

DescriptorSet::Layout& DescriptorSet::Layout::addRange(Type type, uint baseRegisterIndex, uint descriptorCount, uint registerSpace) {
  Range r;
  r.descCount = descriptorCount;
  r.baseRegisterIndex = baseRegisterIndex;
  r.registerSpace = registerSpace;
  r.type = type;

  mRanges.push_back(r);
  return *this;
}

DescriptorSet::sptr_t DescriptorSet::create(const DescriptorPool::sptr_t& pool, const Layout& layout) {
  sptr_t ds = sptr_t(new DescriptorSet(pool, layout));
  return ds->rhiInit() ? ds : nullptr;
}

DescriptorSet::~DescriptorSet() {
  mPool->releaseAllocation(mRhiData);
}


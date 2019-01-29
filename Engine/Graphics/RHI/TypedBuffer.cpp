#include "TypedBuffer.hpp"

void TypedBuffer::uploadGpu() {
  if (!mCpuDirty) return;
  EXPECTS(mData.size() <= mElementCount * mStride);
  updateData(mData, 0, mData.size());
}

void TypedBuffer::set(uint stride, uint count, const void* data) {
  mElementCount = count;
  mStride = stride;
  mData.set(data, count * stride);
  mCpuDirty = true;
}

const ShaderResourceView& TypedBuffer::srv() const {
  if(!mSrv) {
    mSrv = ShaderResourceView::create(*this);
  }

  return *mSrv;
}

const UnorderedAccessView* TypedBuffer::uav() const {
  if(!mUav) {
    mUav = UnorderedAccessView::create(*this);
  }

  return mUav.get();
}

TypedBuffer::sptr_t TypedBuffer::create(u32 stride, u32 eleCount, BindingFlag bindingFlags) {
  sptr_t b = sptr_t(new TypedBuffer(eleCount, stride, bindingFlags));

  if(!b->rhiInit(false)) {
    return nullptr;
  }

  return b;
}

TypedBuffer::TypedBuffer(u32 eleCount, u32 stride, BindingFlag bindingFlags)
  :RHIBuffer(eleCount * stride, bindingFlags, CPUAccess::None)
  , mElementCount(eleCount)
  , mStride(stride) {
  static const uint32_t zero = 0;
  mUavCounter = RHIBuffer::create(sizeof(uint32_t), BindingFlag::UnorderedAccess, CPUAccess::None, &zero);
}

void TypedBuffer::set(const void* data, u32 size, u32 byteOffset) {
  mData.set(data, size, byteOffset);
  mCpuDirty = true;
}

void* TypedBuffer::get(u32 byteOffset) {
  EXPECTS(byteOffset < mData.size());
  byte_t* start = mData;
  mCpuDirty     = true;
  return start + byteOffset;
}

const void* TypedBuffer::get(u32 byteOffset) const {
  EXPECTS(byteOffset < mData.size());
  byte_t* start = mData;

  return start + byteOffset;
}

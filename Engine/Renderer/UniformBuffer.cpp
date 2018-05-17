#include "UniformBuffer.hpp"
#include "glFunctions.hpp"

UniformBuffer::UniformBuffer() {}
UniformBuffer::~UniformBuffer() {
  cleanup();
}

void UniformBuffer::cleanup() {
}

void UniformBuffer::putCpu(size_t byteSize, void const* data) {
  mData = Blob(data, byteSize);
  mDirtyBit = true;
}

void UniformBuffer::putGpu() {
  if (!mDirtyBit) return;
  mRenderBuffer.copyToGpu(mData.size(), mData);
  mDirtyBit = false;
}

void UniformBuffer::put(size_t byteSize, void const* data) {
  putCpu(byteSize, data);
  putGpu();
}

void UniformBuffer::putCpu(unsigned offset, size_t byteSize, const void* data) {

  mData.set(data, byteSize, offset);

  mDirtyBit = true;
}

void* UniformBuffer::get() {
  mDirtyBit = true;
  return mData;
}

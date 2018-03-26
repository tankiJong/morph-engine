#pragma once
#include "Engine/Core/common.hpp"

class RenderBuffer {
public:
  ~RenderBuffer();
  RenderBuffer() = default;
  bool copyToGpu(size_t byteCount, const void* data);
  inline uint handle() const { return mHandle; }
  inline size_t size() const { return mBufferSize; }
protected:
  uint mHandle = 0;
  size_t mBufferSize = 0;
};

 
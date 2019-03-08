#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

class RingBuffer {
public:

  size_t size() const { return mWriteIndex - mReadIndex; }
  bool empty() const { return mReadIndex == mWriteIndex; }
  bool full() const { return size() == kBufferSize; }

  size_t push(const float& val) {
    EXPECTS(!full());
    size_t writeIndex = mask(mWriteIndex++);
    mBuffer[writeIndex] = val; 
    return writeIndex;
  }

  float pop() { EXPECTS(!empty()); return mBuffer[mask(mReadIndex++)]; }

  const size_t offset() { return mReadIndex; }
  const float& front() const { return mBuffer[mask(mReadIndex)]; }

  constexpr size_t capacity() const { return kBufferSize; }
  static constexpr size_t kBufferSize = BIT_FLAG(5);

  static_assert(!(kBufferSize&(kBufferSize - 1)), "size of the ring buffer has to be power of 2");
protected:

  static constexpr size_t mask(size_t val) { return val & (kBufferSize - 1); }
  std::array<float, kBufferSize> mBuffer{};
  size_t mReadIndex = 1;
  size_t mWriteIndex = 1;

};

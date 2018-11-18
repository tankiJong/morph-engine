#pragma once
#include "Engine/Core/common.hpp"

enum eEndianness {
  ENDIANNESS_BIG = 0,
  ENDIANNESS_LITTLE = 1,
};

constexpr eEndianness platformEndianness() {
  const uint32_t v = 1;
  byte_t* b = (byte_t*)&v;

  return (eEndianness)b[0];
}

inline void toEndianness(eEndianness target, void* data, size_t byteSize) {
  if (target == platformEndianness()) return;
  byte_t* begin = (byte_t*)data;
  auto end = begin + byteSize - 1;

  while (begin < end) {
    std::swap(*begin, *end);
    ++begin;
    --end;
  }
}

inline void fromEndianness(eEndianness from, void* data, size_t byteSize) {
  toEndianness(from, data, byteSize);
}

template<size_t N, typename Byte>
inline void toEndianness(eEndianness e, Byte data[N]) {
  static_assert(sizeof(Byte) == 1);
  toEndianness(e, data, N);
}

template<typename T>
inline void toEndianness(eEndianness e, T& data) {
  toEndianness(e, &data, sizeof(T));
}

template<size_t N, typename Byte>
inline void fromEndianness(eEndianness e, Byte data[N]) {
  static_assert(sizeof(Byte) == 1);
  fromEndianness(e, data, N);
}

template<typename T>
inline void fromEndianness(eEndianness e, T& data) {
  fromEndianness(e, &data, sizeof(T));
}
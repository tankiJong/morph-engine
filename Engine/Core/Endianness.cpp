#include "Endianness.hpp"

void toEndianness(eEndianness target, void* data, size_t byteSize) {
  if (target == platformEndianness()) return;
  byte_t* begin = (byte_t*)data;
  auto end = begin + byteSize - 1;

  while (begin < end) {
    std::swap(*begin, *end);
    ++begin;
    --end;
  }
}
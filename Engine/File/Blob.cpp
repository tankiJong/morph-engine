#include "Blob.hpp"
#include "Game/GameCommon.hpp"
Blob::~Blob() {
  free(buffer);
}

Blob::Blob(Blob&& source) noexcept {
  buffer = source.buffer;
  size = source.size;
}

Blob Blob::clone() const {
  void* block = malloc(size);

  memcpy(block, buffer, size);

  return Blob(block, size);
}

Blob& Blob::operator=(Blob&& other) noexcept {
  buffer = other.buffer;
  size = other.size;

  return *this;
}

#include "Blob.hpp"
#include "Game/GameCommon.hpp"
Blob::~Blob() {
  free(buffer);
}

Blob::Blob(Blob&& source) noexcept {
  buffer = source.buffer;
  dataSize = source.dataSize;
  bufferSize = source.bufferSize;

  source.buffer = malloc(0);
  source.dataSize = 0;
  source.bufferSize = 0;
}

Blob Blob::clone() const {
  void* block = malloc(dataSize);

  memcpy(block, buffer, dataSize);

  return Blob(block, dataSize);
}

Blob& Blob::operator=(Blob&& other) noexcept {
  buffer = other.buffer;
  dataSize = other.dataSize;
  bufferSize = other.bufferSize;

  return *this;
}

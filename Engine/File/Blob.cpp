#include "Blob.hpp"
#include "Game/GameCommon.hpp"
Blob::~Blob() {
  free(buffer);
}

Blob::Blob(Blob&& source) noexcept {
  free(buffer);

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

  free(buffer);
  buffer = other.buffer;
  dataSize = other.dataSize;
  bufferSize = other.bufferSize;

  other.buffer = malloc(0);
  other.dataSize = 0;
  other.bufferSize = 0;
  return *this;
}

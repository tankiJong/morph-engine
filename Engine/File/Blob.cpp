#include "Blob.hpp"
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

void Blob::set(const void* data, size_t size, size_t offset) {
  if (dataSize + bufferSize < offset + size) {
    void* newBuffer = malloc(offset + size);
    memcpy(newBuffer, buffer, dataSize);
    free(buffer);
    buffer = newBuffer;
    dataSize = offset + size;
  }

  memcpy((unsigned char*)buffer + offset, data, size);
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

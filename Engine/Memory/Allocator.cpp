#include "Allocator.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
BlockAllocator::BlockAllocator(size_t blockSize, uint prealloc)
  : mBlockSize(blockSize)
  , mBlockPerChunk(blockSize / CHUNK_SIZE) {

  EXPECTS(mBlockSize < CHUNK_SIZE && mBlockSize != 0);

  size_t chunkCount = prealloc * blockSize / CHUNK_SIZE + 1;
  for(size_t i = 0; i < chunkCount; i++) {
    allocChunk();
  }
}

BlockAllocator::~BlockAllocator() {
  for(void*& chunk: mChunks) {
    ::free(chunk);
  }
}

void* BlockAllocator::alloc(size_t size) {
  EXPECTS(size <= mBlockSize);
  UNUSED(size);
  if(mFreeBlocks.empty()) {
    allocChunk();
  }

  ENSURES(!mFreeBlocks.empty());

  void* ptr = mFreeBlocks.top();
  mFreeBlocks.pop();

  return ptr;
}

void BlockAllocator::free(void* ptr) {
  EXPECTS(own(ptr));

  mFreeBlocks.push(ptr);
}

void BlockAllocator::allocChunk() {

  uint8_t *buffer = (uint8_t*)malloc(CHUNK_SIZE);
  mChunks.push_back(buffer);


  for(size_t i = 0; i < mBlockPerChunk; ++i) {
    mFreeBlocks.push(buffer);
    buffer += mBlockSize;
  }
}

bool BlockAllocator::own(void* ptr) const {

  for(void* chunk: mChunks) {
    size_t offset = (uint8_t*)ptr - (uint8_t*)chunk;
    if(offset < CHUNK_SIZE) {
      if(offset % mBlockSize == 0) {
        return true;
      }
    }
  }

  return false;
}

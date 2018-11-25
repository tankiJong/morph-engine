#pragma once

#include "Engine/Core/common.hpp"
#include <vector>
#include <stack>


class Allocator {
public:
  virtual ~Allocator() = default;

private:
  virtual void* alloc(size_t size) = 0;
  virtual void free(void* ptr) = 0;
};


class DefaultAllocator: public Allocator {

public:
    virtual ~DefaultAllocator() override;
private:
  void* alloc(size_t size) override { return malloc(size); };
  void free(void* ptr) override { ::free(ptr); };
};

class BlockAllocator: public Allocator {
  static constexpr size_t CHUNK_SIZE = 64 KB;
public:
  BlockAllocator(size_t blockSize, uint prealloc = 0);
  virtual ~BlockAllocator() override;

  virtual void* alloc(size_t size) override;
  virtual void  free(void* ptr) override;

private:
  void allocChunk();
  bool own(void* ptr) const;

  std::vector<void*> mChunks;
  std::stack<void*> mFreeBlocks;
  const size_t mBlockSize;
  const size_t mBlockPerChunk;
};

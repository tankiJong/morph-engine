#pragma once
#include "Engine/Core/common.hpp"

class RenderBuffer {
  friend class Renderer;
public:
  ~RenderBuffer();
  bool copyToGpu(size_t byteCount, const void* data);

  uint handle = 0;
  size_t bufferSize = 0;

protected:
  RenderBuffer() = default;
};

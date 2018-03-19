#pragma once
#include "Engine/Core/common.hpp"

class RenderBuffer {
  friend class Renderer;
public:
  ~RenderBuffer();
  RenderBuffer() = default;
  bool copyToGpu(size_t byteCount, const void* data);

  uint handle = 0;
  size_t bufferSize = 0;
};

 
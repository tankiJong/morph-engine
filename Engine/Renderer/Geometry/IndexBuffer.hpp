#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"

class IndexBuffer: public RenderBuffer {
public:
  uint indexCount = 0;
  uint indexStride = 1;

  IndexBuffer(uint stride);
  template<typename T>
  static IndexBuffer For() {
    return IndexBuffer(sizeof(T));
  }
};

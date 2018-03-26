#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"

class VertexBuffer: public RenderBuffer {
public:

  VertexBuffer(uint stride);
  uint vertexCount = 0;
  uint vertexStride = 1;

  template<typename T>
  static VertexBuffer For() {
    return VertexBuffer(sizeof(T));
  }
};

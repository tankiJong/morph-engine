#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/Geometry/VertexLayout.hpp"

class VertexBuffer: public RenderBuffer {
public:

  VertexBuffer(uint stride, const VertexLayout* layout);
  uint vertexCount = 0;
  uint vertexStride = 1;
  const VertexLayout* layout = nullptr;
  template<typename T>
  static VertexBuffer For() {
    return VertexBuffer(sizeof(T), VertexLayout::For<T>());
  }
};

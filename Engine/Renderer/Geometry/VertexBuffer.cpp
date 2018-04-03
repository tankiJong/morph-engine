#include "VertexBuffer.hpp"
VertexBuffer::VertexBuffer(uint stride, const VertexLayout* layout)
  : vertexStride(stride)
  , layout(layout){}

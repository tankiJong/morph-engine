#include "Mesh.hpp"

Mesh& Mesh::setVertices(uint stride, uint count, const void* vertices) {
  mVertices.vertexStride = stride;
  mVertices.vertexCount = count;
  mVertices.copyToGpu(stride * count, vertices);
  return *this;
}

Mesh& Mesh::setInstruction(eDrawPrimitive prim, bool useIndices, uint startIdx, uint elemCount) {
  mIns.prim = prim;
  mIns.useIndices = useIndices;
  mIns.startIndex = startIdx;
  mIns.elementCount = elemCount;
  return *this;
}

Mesh& Mesh::resetInstruction() {
  mIns.prim = DRAW_TRIANGES;
  mIns.useIndices = true;
  mIns.startIndex = 0;
  mIns.elementCount = mIndices.indexCount;
  return *this;
}

Mesh& Mesh::setIndices(span<const uint> indices) {
  mIndices.indexStride = sizeof(uint);
  mIndices.indexCount = indices.size();
  mIndices.copyToGpu(sizeof(uint) * indices.size(), indices.data());
  return *this;
}

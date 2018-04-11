#include "Mesh.hpp"
#include "Mesher.hpp"
#include "Engine/File/Utils.hpp"
#include "Engine/File/FileSystem.hpp"

Mesh& Mesh::setVertices(uint streamIndex, uint stride, uint count, const void* vertices) {
  mVertices[streamIndex].vertexStride = stride;
  mVertices[streamIndex].vertexCount = count;
  mVertices[streamIndex].copyToGpu(stride * count, vertices);
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

Mesh::Mesh(const VertexLayout* layout): mIndices(sizeof(uint)), mLayout(layout) {
  mVertices.resize(layout->attributes().size() + 1);
  for(const VertexAttribute& attribute: layout->attributes()) {
    if(mVertices.size() <= attribute.streamIndex) {
      mVertices.resize(attribute.streamIndex + 1);
    }
    attribute.initVertexBuffer(mVertices[attribute.streamIndex]);
  }
}

Mesh& Mesh::setIndices(span<const uint> indices) {
  mIndices.indexStride = sizeof(uint);
  mIndices.indexCount = indices.size();
  mIndices.copyToGpu(sizeof(uint) * indices.size(), indices.data());
  return *this;
}


template<>
ResDef<Mesh> Resource<Mesh>::load(const fs::path& file) {
  Mesher ms;

  EXPECTS(file.extension() == ".obj");
  ms.begin(DRAW_TRIANGES);
  ms.obj(file);
  ms.end();

  return { file.generic_string(), ms.createMesh() };
}

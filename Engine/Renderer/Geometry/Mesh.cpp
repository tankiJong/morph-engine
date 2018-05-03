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

Mesh& Mesh::pushInstruction(eDrawPrimitive prim, bool useIndices, uint startIdx, uint elemCount) {
  mIns.emplace_back();
  draw_instr_t& ins = mIns.back();
  ins.prim = prim;
  ins.useIndices = useIndices;
  ins.startIndex = startIdx;
  ins.elementCount = elemCount;
  return *this;
}

Mesh& Mesh::resetInstruction(uint index) {
  draw_instr_t& ins = mIns.back();
  ins.prim = DRAW_TRIANGES;
  ins.useIndices = true;
  ins.startIndex = 0;
  ins.elementCount = mIndices.indexCount;
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
ResDef<Mesh> Resource<Mesh>::load(const std::string& file) {
  Mesher ms;

  EXPECTS(fs::path(file).extension() == ".obj");
  ms.begin(DRAW_TRIANGES, false);
  ms.obj(file);
  ms.mikkt();
  ms.end();

  return { file, ms.createMesh() };
}

#include "Mesh.hpp"

Mesh& Mesh::setVertices(uint streamIndex, uint stride, uint count, const void* vertices) {
  if (!mVertices[streamIndex]) {
    mVertices[streamIndex] = VertexBuffer::create(stride, count, RHIResource::BindingFlag::VertexBuffer);
  }
  mVertices[streamIndex]->set(stride, count, vertices);
  mVertices[streamIndex]->uploadGpu();
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
  draw_instr_t& ins = mIns[index];
  ins.prim = DRAW_TRIANGES;
  ins.useIndices = true;
  ins.startIndex = 0;
  ins.elementCount = mIndices->elementCount();
  return *this;
}

Mesh::Mesh(const VertexLayout* layout): mLayout(layout) {
  mVertices.resize(layout->attributes().size());
}

Mesh& Mesh::setIndices(span<const uint> indices) {
  if(!mIndices) {
    mIndices = IndexBuffer::For<uint>(indices.size(), RHIResource::BindingFlag::IndexBuffer);
  }
  mIndices->set(indices);
  mIndices->uploadGpu();
  return *this;
}

//
//template<>
//ResDef<Mesh> Resource<Mesh>::load(const std::string& file) {
//  Mesher ms;
//
//  EXPECTS(fs::path(file).extension() == ".obj");
//  ms.begin(DRAW_TRIANGES, false);
//  ms.obj(file);
//  ms.mikkt();
//  ms.end();
//
//  return { file, ms.createMesh() };
//}

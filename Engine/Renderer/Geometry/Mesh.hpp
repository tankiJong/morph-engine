#pragma once
#include "Engine/Core/common.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "Game/Game.hpp"
#include "type.h"
#include "Vertex.hpp"

/**
 * \brief Abstraction of Mesh type.
 *        User should never directly construct a Mesh instance since Mesh class does not make any promise for the vertex type. 
 *        User should use VertexMesh instead.
 */
class Mesh {
public:

  Mesh& setIndices(span<const uint> indices);
  Mesh& setInstruction(eDrawPrimitive prim, bool useIndices, uint startIdx, uint elemCount);
  Mesh& resetInstruction();

  const VertexBuffer& vertices(uint streamIndex) const { return mVertices[streamIndex]; }
  const IndexBuffer& indices() const { return mIndices; }
  const draw_instr_t& instruction() const { return mIns; }
  const VertexLayout& layout() const { return *mLayout; }

protected:
  Mesh(const VertexLayout* layout);
  std::vector<VertexBuffer> mVertices;
  IndexBuffer  mIndices;
  draw_instr_t mIns;
  const VertexLayout* mLayout = nullptr;

  /**
   * \brief the function is just convenient for internal use;
   */
  Mesh& setVertices(uint streamIndex, uint stride, uint count, const void* vertices);
};

template<typename V>
class VertexMesh: public Mesh {
  static_assert(VertexLayout::Valid<V>(), "there is no valid Vertex Type");
public:
  using VertexType = V;
  VertexMesh(): Mesh(VertexLayout::For<V>()) {
  }

  void setVertices(Vertex& vertices) {
    vertex_a_t v = vertices.vertices();
    char** start = (char**)&v;
    for(VertexAttribute attr: mLayout->attributes()) {
      Mesh::setVertices(attr.streamIndex, attr.stride(), vertices.count(),  *(start + attr.offsetInVertexArray));
    }
  }
};
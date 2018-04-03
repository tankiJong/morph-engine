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

  const VertexBuffer& vertices() const { return mVertices; }
  const IndexBuffer& indices() const { return mIndices; }
  const draw_instr_t& instruction() const { return mIns; }

protected:
  Mesh(uint stride, const VertexLayout* layout): mVertices(stride, layout), mIndices(stride) {};
  VertexBuffer mVertices;
  IndexBuffer  mIndices;
  draw_instr_t mIns;

  /**
   * \brief the function is just convenient for internal use;
   */
  Mesh& setVertices(uint stride, uint count, const void* vertices);
};

template<typename V>
class VertexMesh: public Mesh {
  static_assert(VertexLayout::Valid<V>(), "there is no valid Vertex Type");
public:
  using VertexType = V;
  VertexMesh(): Mesh(sizeof(V), VertexLayout::For<V>()) {
  }

  inline void setVertices(span<VertexType> vertices) {
    setVertices(sizeof(VertexType), vertices.size(), vertices.data());
  }

  inline void setVertices(span<Vertex> vertices) {
    std::vector<VertexType> vs;
    vs.reserve(vertices.size());

    for(auto i = 0; i < vertices.size(); ++i) {
      vs.emplace_back(vertices[i].as<VertexType>());
    }
    Mesh::setVertices(sizeof(VertexType), vs.size(), vs.data());
  }
};
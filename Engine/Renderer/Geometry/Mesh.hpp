#pragma once
#include "Engine/Core/common.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "Game/Game.hpp"
#include "type.h"

class Mesh {
public:
  Mesh(uint stride): mVertices(stride), mIndices(stride) {};

  Mesh& setIndices(span<const uint> indices);
  
  Mesh& setVertices(uint stride, uint count, const void* vertices);
  template<typename T> inline void setVertices(span<T> vertices) {
    setVertices(sizeof(T), vertices.size(), vertices.data());
  }

  Mesh& setInstruction(eDrawPrimitive prim, bool useIndices, uint startIdx, uint elemCount);
  Mesh& resetInstruction();

  const VertexBuffer& vertices() const { return mVertices; }
  const IndexBuffer& indices() const { return mIndices; }
  const draw_instr_t& instruction() const { return mIns; }
protected:
  VertexBuffer mVertices;
  IndexBuffer  mIndices;
  draw_instr_t mIns;
};


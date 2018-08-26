#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHI.hpp"
#include "Vertex.hpp"
#include "Engine/Core/Resource.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"
#include "Engine/Graphics/RHI/TypedBuffer.hpp"
struct draw_instr_t {
  eDrawPrimitive prim = DRAW_TRIANGES;
  bool useIndices = true;
  uint startIndex = 0;
  uint elementCount = 0;
};

class RHIContext;

/**
* \brief Abstraction of Mesh type.
*        User should never directly construct a Mesh instance since Mesh class does not make any promise for the vertex type.
*        User should use VertexMesh instead.
*/
class Mesh {
public:

  Mesh & setIndices(span<const uint> indices);
  Mesh& pushInstruction(eDrawPrimitive prim, bool useIndices, uint startIdx, uint elemCount);
  inline Mesh& setInstructions(const std::vector<draw_instr_t>& ins) { mIns = ins; return *this; };
  Mesh& resetInstruction(uint index);

  const VertexBuffer::sptr_t& vertices(uint streamIndex) const { return mVertices[streamIndex]; }
  const IndexBuffer::sptr_t& indices() const { return mIndices; }
  const draw_instr_t& instruction(uint i = 0) const { return mIns[i]; }
  span<const draw_instr_t> instructions() const { return mIns; }
  draw_instr_t& instruction(uint i = 0) { return mIns[i]; }
  const VertexLayout& layout() const { return *mLayout; }
  uint subMeshCount() const { return (uint)mIns.size(); }

  void bindForContext(RHIContext& ctx) const;

  std::vector<VertexBuffer::sptr_t> mVertices;
  IndexBuffer::sptr_t  mIndices;
protected:
  Mesh(const VertexLayout* layout);
  std::vector<draw_instr_t> mIns;
  const VertexLayout* mLayout = nullptr;

  /**
  * \brief the function is just convenient for internal use;
  */
  Mesh& setVertices(uint streamIndex, uint stride, uint count, const void* vertices);
};

template<typename V>
class VertexMesh : public Mesh {
  static_assert(VertexLayout::Valid<V>(), "there is no valid Vertex Type");
public:
  using VertexType = V;
  VertexMesh() : Mesh(VertexLayout::For<V>()) {}

  void setVertices(Vertex& vertices) {
    vertex_a_t v = vertices.vertices();
    char** start = (char**)&v;
    for (VertexAttribute attr : mLayout->attributes()) {
      Mesh::setVertices(attr.streamIndex, attr.stride(), vertices.count(), *(start + attr.offsetInVertexArray));
    }
  }
};

//template<>
//ResDef<Mesh> Resource<Mesh>::load(const std::string& file);

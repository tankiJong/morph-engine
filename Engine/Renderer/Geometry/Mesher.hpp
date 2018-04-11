#pragma once
#include "Engine/Core/common.hpp"
#include "Game/Game.hpp"
#include "Vertex.hpp"
#include "type.h"
#include "Mesh.hpp"
#include "Engine/File/Path.hpp"

class Mesh;
class Font;
class Mesher {
public:
  Mesher& begin(eDrawPrimitive prim, bool useIndices = true);
  void end(bool reNormal = false);
  void clear();
  Mesher& color(const Rgba& c);
  Mesher& normal(const vec3& n);
  Mesher& uv(const vec2& uv);
  Mesher& uv(float u, float v);

  uint vertex3f(const vec3& pos);
  uint vertex3f(span<const vec3> verts);
  uint vertex3f(float x, float y, float z);
  uint vertex2f(const vec2& pos);

  Mesher& line(const vec3& from, const vec3& to);
  Mesher& sphere(const vec3& center, float size, uint levelX = 10u, uint levelY = 10u);
  Mesher& triangle();
  Mesher& triangle(uint a, uint b, uint c);
  Mesher& quad();
  Mesher& quad(uint a, uint b, uint c, uint d);
  Mesher& quad(const vec3& a, const vec3& b, const vec3& c, const vec3& d);
  Mesher& cube(const vec3& center, const vec3& dimension);

  Mesher& text(const span<const std::string_view> asciiTexts, float size, const Font* font, const vec3& position, const vec3& right = vec3::right, const vec3& up = vec3::up);
  void obj(fs::path objFile);
  template<typename VertexType=vertex_lit_t>
  owner<Mesh*> createMesh();

protected:
  vec3 normalOf(uint a, uint b, uint c);
  vertex_t mStamp;
  Vertex mVertices;
  std::vector<uint> mIndices;
  draw_instr_t mIns;
  bool isDrawing = false;
};

template< typename VertexType >
owner<Mesh*> Mesher::createMesh() {
  GUARANTEE_OR_DIE(isDrawing == false, "createMesh called without calling end()");
  VertexMesh<VertexType>* m = new VertexMesh<VertexType>();
  m->setInstruction(mIns.prim, mIns.useIndices, mIns.startIndex, mIns.elementCount);

  m->setVertices(mVertices);
  if (mIns.useIndices) {
    m->setIndices({ mIndices.data() + mIns.startIndex, mIndices.data() + mIns.elementCount });
  }

  return m;
}


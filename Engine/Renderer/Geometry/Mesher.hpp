#pragma once
#include "Engine/Core/common.hpp"
#include "Game/Game.hpp"
#include "Vertex.hpp"
#include "type.h"
#include "Mesh.hpp"
#include "Engine/File/Path.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/Primitives/FloatRange.hpp"

class Mesh;
class Font;
class Mesher {
  friend class MikktBinding;
public:
  Mesher& begin(eDrawPrimitive prim, bool useIndices = true);
  void end();
  void clear();
  Mesher& color(const Rgba& c);
  Mesher& normal(const vec3& n);
  Mesher& tangent(const vec3& t, float fSign = 1.f);
  Mesher& uv(const vec2& uv);
  Mesher& uv(float u, float v);

  uint vertex3f(const vec3& pos);
  uint vertex3f(span<const vec3> verts);
  uint vertex3f(float x, float y, float z);
  uint vertex2f(const vec2& pos);

  Mesher& genNormal();
  Mesher& line(const vec3& from, const vec3& to);
  Mesher& sphere(const vec3& center, float size, uint levelX = 10u, uint levelY = 10u);
  Mesher& triangle();
  Mesher& triangle(uint a, uint b, uint c);
  Mesher& quad();
  Mesher& quad(uint a, uint b, uint c, uint d);
  Mesher& quad(const vec3& a, const vec3& b, const vec3& c, const vec3& d);
  Mesher& cube(const vec3& center, const vec3& dimension);
  Mesher& cone(const vec3& origin, const vec3& direction, float length, float angle, uint slide = 10, bool bottomFace= true);
  Mesher& text(const span<const std::string_view> asciiTexts, float size, const Font* font, const vec3& position, const vec3& right = vec3::right, const vec3& up = vec3::up);
  void obj(fs::path objFile);
  void surfacePatch(const delegate<vec3(const vec2&)>& parametric);
  void surfacePatch(const FloatRange& u, const FloatRange& v, uint levelX, uint levelY, const delegate<vec3(const vec2&)>& parametric);
  void mikkt();
  template<typename VertexType=vertex_lit_t>
  owner<Mesh*> createMesh();

protected:
  vec3 normalOf(uint a, uint b, uint c);
  uint currentElementCount() const;
  vertex_t mStamp;
  Vertex mVertices;
  std::vector<uint> mIndices;
  std::vector<draw_instr_t> mIns;
  draw_instr_t mCurrentIns;
  bool isDrawing = false;
};

template< typename VertexType >
owner<Mesh*> Mesher::createMesh() {
  GUARANTEE_OR_DIE(isDrawing == false, "createMesh called without calling end()");
  VertexMesh<VertexType>* m = new VertexMesh<VertexType>();
  m->setInstructions(mIns);

  m->setVertices(mVertices);
  m->setIndices(mIndices);

  return m;
}


#pragma once
#include "Engine/Core/common.hpp"
#include "Vertex.hpp"
#include "Engine/File/Path.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/Primitives/FloatRange.hpp"
#include "Engine/Graphics/Model/Mesh.hpp"
#include "Engine/Memory/Pool.hpp"

class aabb2;
class Mesh;
class Font;
class BVH;

class Mesher {
  friend class MikktBinding;
public:
  Mesher & begin(eDrawPrimitive prim, bool useIndices = true);
  void end();
  void clear();
  Mesher& color(const Rgba& c);
  Mesher& color(const vec4& c);
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
  Mesher& line2(const vec2& from, const vec2& to, float z = 0.f);
  Mesher& sphere(const vec3& center, float size, uint levelX = 10u, uint levelY = 10u);
  Mesher& triangle();
  Mesher& triangle(uint a, uint b, uint c);
  Mesher& quad();
  Mesher& quad(uint a, uint b, uint c, uint d);
  inline span<const vec3> normals() { return { mVertices.vertices().normal, (span<vec3>::index_type)mVertices.count() }; }

  // d -- c
  // |    |
  // a -- b
  Mesher& quad(const vec3& a, const vec3& b, const vec3& c, const vec3& d);

  // d -- c
  // |    |
  // a -- b
  Mesher& quad(const vec3& a, const vec3& b, const vec3& c, const vec3& d,
               const vec2& uva, const vec2& uvb, const vec2& uvc, const vec2& uvd);

  // d -- c
  // |    |
  // a -- b
  Mesher& quad(const vec3& center, const vec3& xDir, const vec3& yDir, const vec2& size);

  // d -- c
  // |    |
  // a -- b
  Mesher& quad(const vec3& center, const vec3& xDir, const vec3& yDir, const vec2& size,
               const vec2& uva, const vec2& uvb, const vec2& uvc, const vec2& uvd);

  Mesher& quad2(const aabb2& bound, float z = 0);
  Mesher& cube(const vec3& center, const vec3& dimension);
  Mesher& cube(const vec3& origin, const vec3& dimension, 
               const vec3& right, const vec3& up, const vec3& forward);
  Mesher& cone(const vec3& origin, const vec3& direction, float length, float angle, uint slide = 10, bool bottomFace = true);
  Mesher& text(const span<const std::string_view> asciiTexts, float size, const Font* font,
               const vec3& position, const vec3& right = vec3::right, const vec3& up = vec3::up);
  Mesher& text(const std::string_view asciiText, float size, const Font* font,
               const vec3& position, const vec3& right = vec3::right, const vec3& up = vec3::up);
  Mesher& text(const span<const std::string> asciiTexts, float size, const Font* font,
               const vec3& position, const vec3& right = vec3::right, const vec3& up = vec3::up);
  void obj(fs::path objFile);
  void surfacePatch(const delegate<vec3(const vec2&)>& parametric, float eps = .001f);
  void surfacePatch(const FloatRange& u, const FloatRange& v, uint levelX, uint levelY, const std::function<vec3(const vec2&)>& parametric, float eps = .001f);
  void surfacePatch(const std::function<vec3(const vec2&, const ivec2&)>& parametric, const FloatRange& u, const FloatRange& v, uint levelX, uint levelY, float eps = .001f);
  void mikkt();
  template<typename VertexType = vertex_lit_t>
  owner<Mesh*> createMesh();
  owner<BVH*> createBVH(uint maxDepth);
  template<typename VertexType = vertex_lit_t>
  void resetMesh(Mesh& mesh);

  Vertex mVertices;
  std::vector<uint> mIndices;
  std::vector<draw_instr_t> mIns;
protected:
  vec3 normalOf(uint a, uint b, uint c);
  uint currentElementCount() const;
  vertex_t mStamp;
  draw_instr_t mCurrentIns;
  bool isDrawing = false;

};

template< typename VertexType >
owner<Mesh*> Mesher::createMesh() {
  GUARANTEE_OR_DIE(isDrawing == false, "createMesh called without calling end()");
  VertexMesh<VertexType>* m = VertexMesh<VertexType>::pool.acquire();
  
  m->setInstructions(mIns);
  m->setVertices(mVertices);
  
  if(mCurrentIns.useIndices) {
    m->setIndices(mIndices);
  }

  return m;
}

template< typename VertexType >
void Mesher::resetMesh(Mesh& mesh) {
  GUARANTEE_OR_DIE(isDrawing == false, "createMesh called without calling end()");
  VertexMesh<VertexType>* m = reinterpret_cast<VertexMesh<VertexType>*>(&mesh);
  m->setInstructions(mIns);

  m->setVertices(mVertices);
  m->setIndices(mIndices);
}

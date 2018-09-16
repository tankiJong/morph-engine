#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Graphics/RHI/VertexLayout.hpp"

DeclVertexType(vertex_pcu_t) {
  vec3 position{ 0.f };
  vec4 color{ 1.f, 1.f, 1.f, 1.f };
  vec2 uvs{ 0.f };

  vertex_pcu_t() = default;
  vertex_pcu_t(const vec3& pos, const Rgba& col, const vec2& uvs) : position(pos), color(col.normalized()), uvs(uvs) {}
  vertex_pcu_t(const vec2& pos, const Rgba& col, const vec2& uvs) : position(pos), color(col.normalized()), uvs(uvs) {}
};



DeclVertexType(vertex_lit_t) {
  vec3 position{ 0.f };
  vec4 color{ 1.f, 1.f, 1.f, 1.f };
  vec2 uvs{ 0.f };

  vec3 normal;
  vec3 tangent;
};

struct vertex_t {
  vec3 position;
  vec4 color{ 1.f, 1.f, 1.f, 1.f };
  vec2 uv;

  vec3 normal;
  vec4 tangent;
};

struct vertex_a_t {
  vec3* position;
  vec4* color;
  vec2* uv;

  vec3* normal;
  vec4* tangent;
};
class Vertex {
public:
  Vertex();

  vertex_a_t vertices();

  void push(vertex_t v);

  inline uint count() const { return mCount; }

  void reserve(uint size);
  void clear();
protected:
  std::vector<vec3> mPositions;
  std::vector<vec4> mColors;
  std::vector<vec2> mUVs;
  std::vector<vec3> mNormals;
  std::vector<vec4> mTangents;
  uint mCount = 0;
};

#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "VertexLayout.hpp"

DeclVertexType(vertex_pcu_t) {
  vec3 position{ 0.f };
  Rgba color{ 255, 255, 255, 255 };
  vec2 uvs{ 0.f };

  vertex_pcu_t() = default;
  vertex_pcu_t(const vec3& pos, const Rgba& col, const vec2& uvs) : position(pos), color(col), uvs(uvs) {}
  vertex_pcu_t(const vec2& pos, const Rgba& col, const vec2& uvs) : position(pos), color(col), uvs(uvs) {}
};



DeclVertexType(vertex_lit_t) {
  vec3 position{ 0.f };
  Rgba color{ 255, 255, 255, 255 };
  vec2 uvs{ 0.f };

  vec3 normal;
};

struct vertex_t {
  vec3 position;
  Rgba color;
  vec2 uv;
};

struct vertex_a_t {
  vec3* position;
  Rgba* color;
  vec2* uv;
};
class Vertex {
public:
  Vertex();

  vertex_a_t vertices();
//  /**
//   * \brief Do not have definition to prevent abuse usage, like pasing in a random type
//   */
  void push(vertex_t v);

  inline uint count() const { return mCount; }

  void reserve(uint size);
  void clear();
protected:
  std::vector<vec3> mPositions;
  std::vector<Rgba> mColors;
  std::vector<vec2> mUVs;
  uint mCount = 0;
};

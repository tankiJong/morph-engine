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

class Vertex {
public:
  vec3 position;
  Rgba color;
  vec2 uv;

  template<typename VertexType>
  VertexType as();
};


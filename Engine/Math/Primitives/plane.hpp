#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec3.hpp"

struct plane_t {
  vec3 normal;
  float distance;

  plane_t(const vec3& a, const vec3& b, const vec3& c);
  bool inFront(const vec3& pos) const;
};

#pragma once
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Math/Primitives/aabb3.hpp"

struct sphere {
  vec3 center;
  float radius;

  inline bool overlap(const sphere& rhs) const {
    float rr = radius + rhs.radius;
    return (center - rhs.center).magnitude2() < rr*rr;
  };

  bool overlap(const aabb3& box) const;
};

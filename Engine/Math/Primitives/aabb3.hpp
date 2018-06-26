#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec3.hpp"

struct aabb3 {
  aabb3()
    : min(INFINITY)
    , max(-INFINITY) {}
  aabb3(const vec3& center, float size);
  aabb3(const vec3& center, const vec3& size);

  /*
  *   7 ----- 6
  *  / |    / |
  * 4 -+-- 5  |
  * |  3 --|- 2
  * | /    | /
  * 0 ---- 1
  *
  */
  void corners(span<vec3> out) const;
  vec3 corner(uint index) const;
  inline vec3 center() const { return (min + max) * .5f; }

  inline vec3 size() const { return max - min; };
  inline vec3 halfExtent() const { return (max - min) * .5f; }
  inline void invalidate() {
    min = vec3(INFINITY);
    max = vec3(-INFINITY);
  };

  inline bool valid() const { return max.x >= min.x; }

  bool contains(const vec3& pos);
  bool grow(const vec3& pos);
  bool grow(const aabb3& bound);

  vec3 min;
  vec3 max;
};

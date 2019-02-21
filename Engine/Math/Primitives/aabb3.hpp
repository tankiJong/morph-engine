#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec3.hpp"

#undef min

#undef max

struct aabb3 {
  aabb3()
    : mins(INFINITY)
    , maxs(-INFINITY) {}
  aabb3(const vec3& mins, const vec3& maxs)
  : mins(mins), maxs(maxs) {}
  aabb3(const vec3& center, float size);

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
  inline vec3 center() const { return (mins + maxs) * .5f; }

  inline vec3 size() const { return maxs - mins; };
  inline vec3 halfExtent() const { return (maxs - mins) * .5f; }
  inline void invalidate() {
    mins = vec3(INFINITY);
    maxs = vec3(-INFINITY);
  };

  inline bool valid() const { return maxs.x >= mins.x; }

  bool contains(const vec3& pos) const;
  bool grow(const vec3& pos);
  bool grow(const aabb3& bound);

  vec3 mins;
  vec3 maxs;
};

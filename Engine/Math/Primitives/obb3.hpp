#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/mat44.hpp"

struct obb3 {
  bool contains(const vec3& pos);

  /*
   *   8 ----- 7
   *  / |    / |
   * 5 -+-- 6  |
   * |  4 --|- 3
   * | /    | /
   * 1 ---- 2
   * 
   */
  void corners(span<vec3> out);
protected:
  mat44 space;
  mat44 inverseSpace;
};

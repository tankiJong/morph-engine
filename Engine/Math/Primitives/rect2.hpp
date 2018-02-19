#pragma once
#include "Engine/Core/common.hpp"
#include "vec2.hpp"

class aabb2;
class rect2 {
public:
  vec2 p, q;
  rect2(float px, float py, float qx, float qy);
  rect2(aabb2 aabb);
  const static rect2 zero_one;

  std::array<vec2, 4> vertices() const;
};

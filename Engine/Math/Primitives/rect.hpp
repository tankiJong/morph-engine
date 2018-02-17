#pragma once
#include "Engine/Core/common.hpp"
#include "Vec2.hpp"

class aabb2;
class rect {
public:
  vec2 p, q;
  rect(float px, float py, float qx, float qy);
  rect(aabb2 aabb);
  const static rect zero_one;

  std::array<vec2, 4> vertices() const;
};

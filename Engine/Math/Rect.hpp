#pragma once
#include "Engine/Core/common.hpp"
#include "Vec2.hpp"

class rect {
public:
  vec2 p, q;
  rect(float px, float py, float qx, float qy);
  const static rect zero_one;

  std::array<vec2, 4> vertices() const;
};

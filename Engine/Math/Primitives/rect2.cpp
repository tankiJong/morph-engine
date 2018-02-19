#include "rect2.hpp"
#include <array>
#include "AABB2.hpp"

const rect2 rect2::zero_one{0,0,1,1};

rect2::rect2(float px, float py, float qx, float qy) 
  : p(px, py)
  , q(qx, qy) {}

rect2::rect2(aabb2 aabb)
  : p(aabb.mins), q(aabb.maxs) {}

std::array<vec2, 4> rect2::vertices() const {
  return {
    p,
    { q.x, p.y },
    q,
    { p.x, q.y },
  };
}

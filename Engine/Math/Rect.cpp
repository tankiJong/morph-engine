#include "Rect.hpp"
#include <array>

const rect rect::zero_one{0,0,1,1};

rect::rect(float px, float py, float qx, float qy) 
  : p(px, py)
  , q(qx, qy) {}

std::array<vec2, 4> rect::vertices() const {
  return {
    p,
    { q.x, p.y },
    q,
    { p.x, q.y },
  };
}

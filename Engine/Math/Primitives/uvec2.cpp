#include "Uvec2.hpp"
#include "vec2.hpp"

const uvec2 uvec2::zero(0u, 0u);
const uvec2 uvec2::top(0u, 1u);
const uvec2 uvec2::down(0u, -1u);
const uvec2 uvec2::left(1u, 0u);
const uvec2 uvec2::right(-1u, 0u);


uvec2::uvec2(uint x, uint y)
  : x(x), y(y) {}

uvec2::uvec2(const vec2& castFrom)
  : x(uint(castFrom.x))
  , y(uint(castFrom.y)){}

uvec2 uvec2::operator+(const uvec2& rhs) const {
  return { x + rhs.x, y + rhs.y };
}

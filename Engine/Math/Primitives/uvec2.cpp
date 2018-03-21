#include "uvec2.hpp"
#include "vec2.hpp"
#include "ivec2.hpp"
#include "Game/Utils/Resource.hpp"

const uvec2 uvec2::zero(0u, 0u);
const uvec2 uvec2::top(0u, 1u);
const uvec2 uvec2::down(0u, uint(-1));
const uvec2 uvec2::left(1u, 0u);
const uvec2 uvec2::right(uint(-1), 0u);


uvec2::uvec2(uint x, uint y)
  : x(x), y(y) {}

uvec2::uvec2(const vec2& castFrom)
  : x(uint(castFrom.x))
  , y(uint(castFrom.y)){}

uvec2::uvec2(const ivec2& castFrom)
  : x(uint(castFrom.x))
  , y(uint(castFrom.y)) {}

uvec2 uvec2::operator+(const uvec2& rhs) const {
  return { x + rhs.x, y + rhs.y };
}

uvec2 uvec2::operator-(const uvec2& rhs) const {
  return { x - rhs.x, y - rhs.y };
}

bool uvec2::operator==(const uvec2& rhs) const {
  return x == rhs.x && y == rhs.y;
}

bool uvec2::operator!=(const uvec2& rhs) const {
  return !(rhs == *this);
}

void uvec2::fromString(const char* data) {
  auto raw = split(data, " ,");
  GUARANTEE_OR_DIE(raw.size() == 2, "illegal input string to parse");

  x = parse<uint>(raw[0]);
  y = parse<uint>(raw[1]);
}

uint uvec2::manhattan() const {
  return x + y;
}

uint uvec2::manhattan(const uvec2& rhs) const {
  return std::max(x, rhs.x) - std::min(x, rhs.x)
       + std::max(y, rhs.y) - std::min(y, rhs.y);
}

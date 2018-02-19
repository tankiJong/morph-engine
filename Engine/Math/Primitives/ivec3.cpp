#include "Ivec3.hpp"
#include "ivec2.hpp"
#include "uvec3.hpp"

ivec3::ivec3(int x, int y, int z)
  : x(x), y(y), z(z) {
}

ivec3::ivec3(ivec2 xy, int z)
  : x(xy.x), y(xy.y), z(z) {}

ivec3::ivec3(const uvec3& from)
  : x((int)from.x), y((int)from.y), z((int)from.z) {}


ivec2 ivec3::xy() const {
  return { x, y };
}

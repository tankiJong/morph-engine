#include "Ivec3.hpp"
#include "ivec2.hpp"

ivec3::ivec3(int x, int y, int z)
  : x(x), y(y), z(z) {
}

ivec3::ivec3(ivec2 xy, int z)
  : x(xy.x), y(xy.y), z(z) {}

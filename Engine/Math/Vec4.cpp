#include "vec4.hpp"

const vec4 vec4::right  (1, 0, 0, 0);
const vec4 vec4::up     (0, 1, 0, 0);
const vec4 vec4::forward(0, 0, 1, 0);
const vec4 vec4::zero   (0, 0, 0, 0);

vec4::vec4(const vec3& copy) 
  : x(copy.x)
  , y(copy.y)
  , z(copy.z) 
  , w(0) {}

vec4::vec4(const vec3& xyz, float w)
  : x(xyz.x)
  , y(xyz.y)
  , z(xyz.z)
  , w(w) {}

bool vec4::operator==(const vec4& rhs) const {
  return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

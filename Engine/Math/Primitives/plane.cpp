#include "plane.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
plane_t::plane_t(const vec3& a, const vec3& b, const vec3& c) {
  // ccw is a normal point torward us
  vec3 e0 = b - a;
  vec3 e1 = c - a;

  normal = e0.cross(e1);
  normal.normalize();
  distance = normal.dot(a);
}

#include "obb3.hpp"

bool obb3::contains(const vec3& pos) {
  vec3 local = (inverseSpace * vec4(pos, 1.f)).xyz();
  return vec3::abs(local) <= vec3::one;
}

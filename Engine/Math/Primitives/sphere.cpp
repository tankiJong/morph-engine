#include "sphere.h"
#include "Engine/Math/MathUtils.hpp"
bool sphere::overlap(const aabb3& box) const {
  vec3 halfExtent = box.halfExtent();
  vec3 difference = center - box.center();

  vec3 clamped = clamp(difference, -halfExtent, halfExtent);

  vec3 closet = clamped + center;

  return closet.distance2(center) < radius * radius;
}

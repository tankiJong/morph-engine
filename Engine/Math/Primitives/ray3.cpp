#include "ray3.hpp"
#include "Engine/Physics/contact3.hpp"
#include "Engine/Math/Primitives/aabb3.hpp"
#include "Engine/Math/Range.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Math/Primitives/plane.hpp"

contact3 ray3::intersect(const aabb3& box) const {
  float rayLength = (box.center() - start).magnitude();
  float len = rayLength / direction.magnitude() * 2.f;

  vec3 vec = direction * len;

  vec2 x(box.min.x, box.max.x), y(box.min.y, box.max.y), z(box.min.z, box.max.z);

  if(direction.x < 0) {
    std::swap(x.x, x.y);
  }

  if (direction.y < 0) {
    std::swap(y.x, y.y);
  }

  if (direction.z < 0) {
    std::swap(z.x, z.y);
  }

  Range<float> tx((x.x - start.x) / vec.x, (x.y - start.x) / vec.x);
  Range<float> ty((y.x - start.y) / vec.y, (y.y - start.y) / vec.y);
  Range<float> tz((z.x - start.z) / vec.z, (z.y - start.z) / vec.z);

  Range<float> result = tx.intersection(ty).intersection(tz);
  if(result.empty()) {
    return contact3();
  }
  contact3 re;
  re.position = evaluate(result.min*len);

  vec3 center = box.center();
  vec3 dir = re.position - center;

  vec3 corners[8];
  box.corners(corners);

  uint index[3];
  uint p = 0;
  for(uint i = 0; i < 8 && p < 3; ++i) {
    if(dir.dot(corners[i] - center) > 0) {
      index[p++] = i;
    }
  }

  plane_t plane(corners[index[0]], corners[index[1]], corners[index[2]]);

  re.normal = (dir.dot(plane.normal) > 0) ? plane.normal : -plane.normal;

  return re;
}

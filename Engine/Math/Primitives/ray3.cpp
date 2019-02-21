#include "ray3.hpp"
#include "Engine/Physics/contact3.hpp"
#include "Engine/Math/Primitives/aabb3.hpp"
#include "Engine/Math/Range.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Math/Primitives/plane.hpp"

contact3 ray3::intersect(const aabb3& box) const {
  float rayLength = (box.center() - start).magnitude() * 2;

  vec3 vec = direction * rayLength;

  vec2 x(box.mins.x, box.maxs.x), y(box.mins.y, box.maxs.y), z(box.mins.z, box.maxs.z);

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
  
   float t = std::max(result.mins, result.maxs);
   if (t < 0) return contact3();

  contact3 re;
  bool inside = box.contains(start);
  if(inside) {
    re.position = evaluate(result.maxs * rayLength);
  } else {
    re.position = evaluate(result.mins * rayLength);
  }

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

  float sign = box.contains(start) ? -1.f : 1.f;
  if(re.position.x == box.mins.x) {
    re.normal = -sign * vec3::right;
  }
  if(re.position.x == box.maxs.x) {
    re.normal = sign * vec3::right;
  }

  if(re.position.y == box.mins.y) {
    re.normal = -sign * vec3::up;
  }
  if(re.position.y == box.maxs.y) {
    re.normal = sign * vec3::up;
  }

  if(re.position.z == box.mins.z) {
    re.normal = -sign * vec3::forward;
  }
  if(re.position.z == box.maxs.z) {
    re.normal = sign * vec3::forward;
  }
  //
  // plane_t plane(corners[index[0]], corners[index[1]], corners[index[2]]);
  //
  // re.normal = (dir.dot(plane.normal) > 0) ? plane.normal : -plane.normal;

  return re;
}

contact3 ray3::intersect(const sphere& ball) const {
  float a = direction.magnitude2();
  float b = 2 * (
      direction.x * (start.x - ball.center.x)
    + direction.y * (start.y - ball.center.y)
    + direction.z * (start.z - ball.center.z));

  float c = start.distance2(ball.center) - ball.radius * ball.radius;

  if (b*b - 4 * a*c < 0) return contact3();

  float t = (-b + sqrtf(b*b - 4 * a*c)) / (2 * a);

  contact3 cc;
  cc.position = evaluate(t);
  cc.normal = (cc.position - ball.center).normalized();

  return cc;
}

#include "frustum.hpp"
#include "Engine/Math/Primitives/aabb3.hpp"
#include "Engine/Math/Primitives/mat44.hpp"

bool frustum_t::contains(const vec3& pos) const {
  for(uint i = 0; i < 6; ++i) {
    if(!planes[i].inFront(pos)) {
      return false;
    }
  }
  return true;
}

frustum_t frustum_t::fromMat(const mat44& worldToNdc) {
  aabb3 ndc = aabb3(vec3::zero, 2.f);

  vec3 corners[8];
  ndc.corners(corners);

  mat44 inverse = worldToNdc.inverse();

  vec3 worldCorners[8];

  for(uint i = 0; i<8; ++i) {
    vec4 ndcPos = vec4(corners[i], 1.f);
    vec4 kindaWorld = inverse * ndcPos;

    worldCorners[i] = kindaWorld.xyz() / kindaWorld.w;
  }

  frustum_t frustum;
  frustum.left    = plane_t(worldCorners[0], worldCorners[4], worldCorners[2]);
  frustum.right   = plane_t(worldCorners[0], worldCorners[4], worldCorners[2]);
  frustum.bottom  = plane_t(worldCorners[0], worldCorners[4], worldCorners[2]);
  frustum.top     = plane_t(worldCorners[0], worldCorners[4], worldCorners[2]);
  frustum.back    = plane_t(worldCorners[0], worldCorners[4], worldCorners[2]);
  frustum.font    = plane_t(worldCorners[0], worldCorners[4], worldCorners[2]);
}

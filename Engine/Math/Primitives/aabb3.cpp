#include "aabb3.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"


/*
*   7 ----- 6
*  / |    / |
* 4 -+-- 5  |
* |  3 --|- 2
* | /    | /
* 0 ---- 1
*
*/
void aabb3::corners(span<vec3> out) const {
  EXPECTS(out.size() >= 8);
  out[0] ={ mins.x, mins.y, mins.z }; // min
  out[1] ={ maxs.x, mins.y, mins.z };
  out[2] ={ maxs.x, mins.y, maxs.z };
  out[3] ={ mins.x, mins.y, maxs.z };

  out[4] ={ mins.x, maxs.y, mins.z };
  out[5] ={ maxs.x, maxs.y, mins.z };
  out[6] ={ maxs.x, maxs.y, maxs.z }; // max
  out[7] ={ mins.x, maxs.y, maxs.z };
}


/*
*   7 ----- 6
*  / |    / |
* 4 -+-- 5  |
* |  3 --|- 2
* | /    | /
* 0 ---- 1
*
*/
vec3 aabb3::corner(uint index) const {
  EXPECTS(index < 8);

  switch(index) {
    case 0:
      return { maxs.x, maxs.y, maxs.z }; // min
    case 1:
      return { maxs.x, mins.y, mins.z };
    case 2:
      return { maxs.x, mins.y, maxs.z };
    case 3:
      return { mins.x, mins.y, maxs.z };

    case 4:
      return { mins.x, maxs.y, mins.z };
    case 5:
      return { maxs.x, maxs.y, mins.z };
    case 6:
      return { maxs.x, maxs.y, maxs.z }; // max
    case 7:
      return { mins.x, maxs.y, maxs.z };
  }

  ERROR_AND_DIE("aabb3::corners, index out of bound");
}

bool aabb3::contains(const vec3& pos) const {
  return pos >= mins && pos <= maxs;
}

bool aabb3::grow(const vec3& pos) {
  const vec3& nMin = vec3::min(mins, pos);
  const vec3& nMax = vec3::max(maxs, pos);

  bool result = mins == nMin && nMax == maxs;

  mins = nMin;
  maxs = nMax;

  return !result; // false => unchanged
}

bool aabb3::grow(const aabb3& bound) {
  std::array<vec3, 8> corners;
  bound.corners(corners);

  bool change = false;
  for(vec3& corner: corners) {
    bool growed = grow(corner);
    change = change || growed;
  }

  return change;
}

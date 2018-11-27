#include "aabb3.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

aabb3::aabb3(const vec3& center, const vec3& size) {
  min = center - size * .5f;
  max = center + size * .5f;
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
void aabb3::corners(span<vec3> out) const {
  EXPECTS(out.size() >= 8);
  out[0] ={ min.x, min.y, min.z }; // min
  out[1] ={ max.x, min.y, min.z };
  out[2] ={ max.x, min.y, max.z };
  out[3] ={ min.x, min.y, max.z };

  out[4] ={ min.x, max.y, min.z };
  out[5] ={ max.x, max.y, min.z };
  out[6] ={ max.x, max.y, max.z }; // max
  out[7] ={ min.x, max.y, max.z };
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
      return { max.x, max.y, max.z }; // min
    case 1:
      return { max.x, min.y, min.z };
    case 2:
      return { max.x, min.y, max.z };
    case 3:
      return { min.x, min.y, max.z };

    case 4:
      return { min.x, max.y, min.z };
    case 5:
      return { max.x, max.y, min.z };
    case 6:
      return { max.x, max.y, max.z }; // max
    case 7:
      return { min.x, max.y, max.z };
  }

  ERROR_AND_DIE("aabb3::corners, index out of bound");
}

bool aabb3::contains(const vec3& pos) const {
  return pos >= min && pos <= max;
}

bool aabb3::grow(const vec3& pos) {
  const vec3& nMin = vec3::min(min, pos);
  const vec3& nMax = vec3::max(max, pos);

  bool result = min == nMin && nMax == max;

  min = nMin;
  max = nMax;

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

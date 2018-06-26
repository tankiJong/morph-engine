#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/plane.hpp"

class mat44;
class vec3;

struct frustum_t {
  frustum_t() {};
  union {
    plane_t planes[6];
    struct {
      plane_t left;
      plane_t right;
      plane_t bottom;
      plane_t top;
      plane_t back;
      plane_t font;
    };
  };

  bool contains(const vec3& pos) const;

  static frustum_t fromMat(const mat44& worldToNdc);
};

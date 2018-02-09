#pragma once
#include "Engine/Core/common.hpp"

struct vec4 {
public:
  union {
    struct {
      float x = 0, y = 0, z = 0, w = 0;
    };
    struct {
      float r, g, b, a;
    };
    float data[4];
  };
  vec4() {};
  vec4(float x, float y, float z, float w)
    : x(x)
    , y(y)
    , z(z)
    , w(w) {}
  inline float dot(const vec4& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
  }

  // x, i
  static vec4 right;

  // y, j
  static vec4 top;

  // z, k
  static vec4 forward;
};

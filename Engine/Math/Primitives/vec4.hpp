﻿#pragma once
#include "Engine/Core/common.hpp"

class vec3;
class vec2;
struct vec4 {
public:
  union {
    struct {
      float x, y, z, w;
    };
    struct {
      float r, g, b, a;
    };
    struct {
      float i,j,k,t;
    };

    float data[4];
  };
  vec4() {}
  vec3 xyz() const;
  vec2 xy() const;
  vec4(float x, float y, float z, float w)
    : x(x)
    , y(y)
    , z(z)
    , w(w) {}
  inline float dot(const vec4& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
  }

  explicit vec4(const vec3& copy);
  vec4(const vec3& xyz, float w);
  vec4(const vec2& xy, float z, float w);
  void fromString(const char* data);
  bool operator==(const vec4& rhs) const;
  // x, i
  const static vec4 right;

  // y, j
  const static vec4 up;

  // z, k
  const static vec4 forward;

  const static vec4 zero;

  const static vec4 one;
};

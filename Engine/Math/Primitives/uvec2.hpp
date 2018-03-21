#pragma once
#include "Engine/Core/common.hpp"

class vec2;
class ivec2;
class uvec2 {
public:
  uint x = 0u, y = 0u;
  uvec2() = default;
  uvec2(uint x, uint y);
  explicit uvec2(const vec2& castFrom);
  explicit uvec2(const ivec2& castFrom);
  uvec2 operator+(const uvec2& rhs) const;
  uvec2 operator-(const uvec2& rhs) const;

  bool operator==(const uvec2& rhs) const;
  bool operator!=(const uvec2& rhs) const;

  void fromString(const char* data);

  uint manhattan() const;

  uint manhattan(const uvec2& rhs) const;
  static const uvec2 zero;

  static const uvec2 top;
  static const uvec2 down;
  static const uvec2 left;
  static const uvec2 right;

  
};

#pragma once
#include <string>
#include "ivec3.hpp"

namespace std {
  template< typename T >
  class initializer_list;
}

class vec2;
class vec3 {
public:
  float x = 0;
  float y = 0;
  float z = 0;
  vec3() {}
  ~vec3() {}
  vec3(float x, float y, float z);
  explicit vec3(const vec2& vec); // have to cast by intention
  explicit vec3(const ivec3 castFrom);
  vec3(const vec2& vec, float z); // have to cast by intention
  vec3(const vec3& vec);
  vec3(const char* str);

  void fromString(const char* data);
  std::string toString() const;

  // Operators
  const vec3 operator+(const vec3& vecToAdd) const; // vec3 + vec3
  const vec3 operator-(const vec3& vecToSubtract) const; // vec3 - vec3
  const vec3 operator-() const; // vec3 - vec3
  const vec3 operator*(float uniformScale) const; // vec3 * float
  const vec3 operator/(float inverseScale) const; // vec3 / float
  void operator+=(const vec3& vecToAdd); // vec3 += vec3
  void operator-=(const vec3& vecToSubtract); // vec3 -= vec3
  void operator*=(const float uniformScale); // vec3 *= float
  void operator/=(const float uniformDivisor); // vec3 /= float
  void operator=(const vec3& copyFrom); // vec3 = vec3
  bool operator==(const vec3& compare) const; // vec3 == vec3
  bool operator!=(const vec3& compare) const; // vec3 != vec3

  float magnitude() const;
  float magnitudeSquared() const;
  float normalizeAndGetMagnitude();

  float manhattan() const;
  float manhattan(const vec3& rhs) const;

  vec3 normalized() const;
  float dot(const vec3& rhs) const;
  vec3 cross(const vec3& rhs) const;
  vec2 xy() const;
  vec2 xz() const;
  static float dot(const vec3& a, const vec3& b);
  static vec3 fromSpherical(float r, float thetaDeg, float phiDeg);

  // x, i
  const static vec3 right;

  // y, j
  const static vec3 up;

  // z, k
  const static vec3 forward;

  static const vec3 zero;
  static const vec3 one;
};


vec3 operator*(float lhs, const vec3& rhs);

vec3 fromSpherical(float r, float thetaDeg, float phiDeg);
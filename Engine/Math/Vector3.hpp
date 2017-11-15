#pragma once
#include <string>

namespace std {
  template< typename T >
  class initializer_list;
}

class Vector2;
class Vector3 {
public:
  float x = 0;
  float y = 0;
  float z = 0;
  Vector3() {}
  ~Vector3() {}
  explicit Vector3(float x, float y, float z);
  explicit Vector3(const Vector2& vec); // have to cast by intention
  Vector3(std::initializer_list<float>);
  Vector3(const Vector3& vec);
  Vector3(const char* str);

  void fromString(const char* data);
  std::string toString() const;

  // Operators
  const Vector3 operator+(const Vector3& vecToAdd) const; // vec3 + vec3
  const Vector3 operator-(const Vector3& vecToSubtract) const; // vec3 - vec3
  const Vector3 operator*(float uniformScale) const; // vec3 * float
  const Vector3 operator/(float inverseScale) const; // vec3 / float
  void operator+=(const Vector3& vecToAdd); // vec3 += vec3
  void operator-=(const Vector3& vecToSubtract); // vec3 -= vec3
  void operator*=(const float uniformScale); // vec3 *= float
  void operator/=(const float uniformDivisor); // vec3 /= float
  void operator=(const Vector3& copyFrom); // vec3 = vec3
  bool operator==(const Vector3& compare) const; // vec3 == vec3
  bool operator!=(const Vector3& compare) const; // vec3 != vec3

  float magnitude() const;
  float magnitudeSquared() const;
  float normalizeAndGetMagnitude();
  Vector3 normalized() const;
  float dot(const Vector3& another) const;
  static float dot(const Vector3& a, const Vector3& b);

  static const Vector3 zero;
};



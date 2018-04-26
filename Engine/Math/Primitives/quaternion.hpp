#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec4.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Math/Primitives/mat44.hpp"

class quaternion;

inline vec3 operator*(const vec3& v, const quaternion& q);
inline vec4 operator*(const vec4& v, const quaternion& q);

class quaternion {
public:
  union {
    struct {
      vec3 v;
      float r;
    };
    vec4 data{1.f, 0.f, 0.f, 0.f};
  };

  quaternion(): v(1.f, 0.f, 0.f), r(1.f) { }

  quaternion(float r, float x, float y, float z);
  quaternion(float r, const vec3& v);
  quaternion(const quaternion& q);
  // this only works if the rotation is ZXY rotation
  quaternion(const Euler& euler);

  // from euler
  quaternion(float x, float y, float z);

  void normalize();
  void invert();

  bool isUnit() const;

  float magnitude() const;
  float magnitude2() const;

  quaternion conjugate() const;
  quaternion inverse() const;

  Euler euler() const;
  mat44 mat() const;

  inline vec3 right() const { return vec3::right * (*this); }
  inline vec3 up() const { return vec3::up * (*this); }
  inline vec3 forward() const { return vec3::forward * (*this); }

  float dot(const quaternion& rhs) const;
  quaternion difference(const quaternion& rhs) const;

  quaternion operator-() const;
  quaternion operator*(const quaternion& rhs) const;
  bool operator==(const quaternion& rhs) const;
  bool operator!=(const quaternion& rhs) const;

  quaternion& operator=(const quaternion& rhs);
  static const quaternion identity;

  static quaternion around(const vec3& axis, float degree);
  static quaternion lookAt(const vec3& forward);
  static float angle(const quaternion& from, const quaternion& to);
};


inline vec3 operator*(const vec3& v, const quaternion& q) {
  quaternion p = quaternion(0.0f, v);
  quaternion result = q.inverse() * p * q;

  return result.v;
}

inline vec4 operator*(const vec4& v, const quaternion& q) {
  vec3 result = v.xyz() * q;
  return vec4(result, v.w);
}


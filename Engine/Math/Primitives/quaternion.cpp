#include "quaternion.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

quaternion const quaternion::identity;

quaternion::quaternion(float r, float x, float y, float z): data(x, y, z, r) {
  EXPECTS(isUnit());
}

quaternion::quaternion(float r, const vec3& v): data(v, r) {
  EXPECTS(isUnit());
}

quaternion::quaternion(const quaternion& q): data(q.data) {
  EXPECTS(isUnit());
}

quaternion::quaternion(const Euler& euler) {
  vec3 const he = .5f * euler;
  float cx = cosDegrees(he.x);
  float sx = sinDegrees(he.x);
  float cy = cosDegrees(he.y);
  float sy = sinDegrees(he.y);
  float cz = cosDegrees(he.z);
  float sz = sinDegrees(he.z);

  float rr = cx * cy*cz + sx * sy*sz;
  float ix = sx * cy*cz + cx * sy*sz;
  float iy = cx * sy*cz - sx * cy*sz;
  float iz = cx * cy*sz - sx * sy*cz;

  data = vec4(ix, iy, iz, rr);
}

quaternion::quaternion(float x, float y, float z): quaternion(Euler{x, y, z}) {
  EXPECTS(isUnit());
}

void quaternion::normalize() {
  float len2 = magnitude2();

  if (equal(len2, 1.f)) return;

  if(fabs(r) < .9999f) {
    float s = sqrtf(1.f - r * r);
    vec3 vec = v / s;
    vec.normalize();

    v = s * vec;
  } else {
    r = 1.f;
    v = vec3::zero;
  }
}

void quaternion::invert() {
  *this = conjugate();
}

bool quaternion::isUnit() const {
  return equal(this->dot(*this), 1.f);
}

float quaternion::magnitude() const {
  return sqrtf(magnitude2());
}

float quaternion::magnitude2() const {
  return data.dot(data);
}

quaternion quaternion::conjugate() const {
  return quaternion(r, -v);
}

quaternion quaternion::inverse() const {
  return conjugate();
}

Euler quaternion::euler() const {
  return mat().euler();
}

// matrix def related
mat44 quaternion::mat() const {
  return mat44(*this);
}

float quaternion::dot(const quaternion& rhs) const {
  return data.dot(rhs.data);
}

quaternion quaternion::difference(const quaternion& rhs) const {
  return inverse() * rhs;
}

quaternion quaternion::operator-() const {
  return quaternion(-r, -v);
}

quaternion quaternion::operator*(const quaternion& rhs) const {
  quaternion ret;

  ret.r = r * rhs.r - v.dot(rhs.v);
  ret.v = r * rhs.v + rhs.r * v + rhs.v.cross(v);

  return ret;
}

bool quaternion::operator==(const quaternion& rhs) const {
  return (equal(r, rhs.r) && v == rhs.v) || (equal(-r, rhs.r) && -v == rhs.v);
}

bool quaternion::operator!=(const quaternion& rhs) const {
  return !(*this == rhs);
}

quaternion& quaternion::operator=(const quaternion& rhs) {
  this->data = rhs.data;
  return *this;
}

quaternion quaternion::around(const vec3& axis, float degree) {
  float angle = .5f * degree;
  float c = cosDegrees(angle);
  float s = sinf(angle);

  return quaternion(c, s * axis);
}

quaternion quaternion::lookAt(const vec3& forward) {
  return mat44::lookAt(vec3::zero, forward).quat();
}

// does the order matter?
float quaternion::angle(const quaternion& from, const quaternion& to) {
  // so just trying to get real part of the difference
  // inverse of a real part is the real part, so that doesn't change
  // but the imaginary part negates.
  // so need to cmpute that.
  float newReal = from.r * to.r - vec3::dot(-from.v, to.v);

  return 2.f * acosDegrees(newReal);
}



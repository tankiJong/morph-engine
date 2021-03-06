﻿#include "vec3.hpp"
#include "Engine//Math/Primitives/vec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Primitives/ivec3.hpp"

const vec3 vec3::right(1, 0, 0);
const vec3 vec3::up(0, 1, 0);
const vec3 vec3::forward(0, 0, 1);
const vec3 vec3::zero(0, 0, 0);
const vec3 vec3::one(1, 1, 1);

vec3::vec3(float x, float y, float z)
  : x(x)
  , y(y)
  , z(z) {}

vec3::vec3(float v) : x(v), y(v), z(v) {}

vec3::vec3(const vec2& vec)
  : x(vec.x)
  , y(vec.y)
  , z(0) {}

vec3::vec3(const ivec3 castFrom)
  : x(float(castFrom.x))
  , y(float(castFrom.y)) 
  , z(float(castFrom.z)) {}

vec3::vec3(const vec2& vec, float z)
  : x(vec.x)
  , y(vec.y)
  , z(z) {}

vec3::vec3(const char* str) {
  fromString(str);
}
void vec3::fromString(const char* data) {
  auto raw = split(data, " ,");
  GUARANTEE_OR_DIE(raw.size() == 2, "illegal input string to parse");

  x = parse<float>(raw[0]);
  y = parse<float>(raw[1]);
}
std::string vec3::toString() const {
  return Stringf("%f,%f,%f", x, y, z);
}

const vec3 vec3::operator+(const vec3& vecToAdd) const {
  return { vecToAdd.x + x, vecToAdd.y + y, vecToAdd.z + z };
}
const vec3 vec3::operator-(const vec3& vecToSubtract) const {
  return { x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z };
}

const vec3 vec3::operator-() const {
  return { -x, -y, -z };
}

const vec3 vec3::operator*(float uniformScale) const {
  return { x * uniformScale, y * uniformScale, z * uniformScale };
}
const vec3 vec3::operator/(float inverseScale) const {
  float uniformScale = 1.f / inverseScale;
  return *this*uniformScale;
}

const vec3 vec3::operator*(const vec3& rhs) const {
  return { x*rhs.x, y*rhs.y, z*rhs.z };
}

void vec3::operator+=(const vec3& vecToAdd) {
  x += vecToAdd.x; 
  y += vecToAdd.y; 
  z += vecToAdd.z;
}
void vec3::operator-=(const vec3& vecToSubtract) {
  x -= vecToSubtract.x;
  y -= vecToSubtract.y;
  z -= vecToSubtract.z;
}
void vec3::operator*=(const float uniformScale) {
  x *= uniformScale;
  y *= uniformScale;
  z *= uniformScale;
}
void vec3::operator/=(const float uniformDivisor) {
  float scale = 1.f / uniformDivisor;
  x *= scale;
  y *= scale;
  z *= scale;
}
void vec3::operator=(const vec3& copyFrom) {
  x = copyFrom.x;
  y = copyFrom.y;
  z = copyFrom.z;
}

bool vec3::operator==(const vec3& compare) const {
  return x == compare.x && y == compare.y && z == compare.z;
}
bool vec3::operator!=(const vec3& compare) const {
  return x != compare.x || y != compare.y || z != compare.z;
}

bool vec3::operator>=(const vec3& rhs) const {
  return x >= rhs.x && y >= rhs.y && z >= rhs.z;
}

bool vec3::operator<=(const vec3& rhs) const {
  return x <= rhs.x && y <= rhs.y && z <= rhs.z;
}

bool vec3::operator>(const vec3& rhs) const {
  return x > rhs.x && y > rhs.y && z > rhs.z;
}

bool vec3::operator<(const vec3& rhs) const {
  return x < rhs.x && y < rhs.y && z < rhs.z;
}

float vec3::magnitude() const {
  return sqrt(x*x + y*y + z*z);
}
float vec3::magnitude2() const {
  return x*x + y*y + z*z;
}
float vec3::normalize() {
  float len = magnitude();
  EXPECTS(!equal0(len));
  float revLen = 1.f / len;
  x *= revLen;
  y *= revLen;
  z *= revLen;
  return len;
}

float vec3::manhattan() const {
  return ::abs(x) + ::abs(y) + ::abs(z);
}

float vec3::manhattan(const vec3& rhs) const {
  return (*this - rhs).manhattan();
}

vec3 vec3::normalized() const {
  float reverseLen = 1.f / magnitude();
  return { x * reverseLen, y * reverseLen, z * reverseLen };
}
float vec3::dot(const vec3& another) const {
  return dot(*this, another);
}


/**
 * \brief cross Pruduct
 * \return since I use left hand basis, so this should use your left hand, which means vec3::right.cross(vec3::up) == vec3::forward
 */
vec3 vec3::cross(const vec3& c) const {
  const vec3& b = *this;
  return {
    b.y * c.z - b.z * c.y,
    b.z * c.x - b.x * c.z,
    b.x * c.y - b.y * c.x,
  };
}

float vec3::distance2(const vec3& rhs) const {
  return (*this - rhs).magnitude2();
}

float vec3::distance(const vec3& rhs) const {
  return sqrt(distance2(rhs));
}

vec2 vec3::xy() const {
  return { x,y };
}

vec2 vec3::xz() const {
  return { x,z };
}

vec2 vec3::yz() const {
  return { y, z };
}

float vec3::angle(const vec3& rhs) const {
  float l = magnitude() * rhs.magnitude();
  float dt = dot(rhs);
  float deg = asinDegrees(clamp(dt / l, -1.f, 1.f));

  EXPECTS(deg <= 180.f && deg >= -180.f);
  return deg;
}

vec3 vec3::abs(const vec3& vec) {
  return { ::abs(vec.x), ::abs(vec.y), ::abs(vec.z) };
}

float vec3::dot(const vec3& a, const vec3& b) {
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

vec3 vec3::fromSpherical(float r, float thetaDeg, float phiDeg) {
  return {
    r*sinDegrees(thetaDeg)*cosDegrees(phiDeg),
    r*sinDegrees(phiDeg),
    r*cosDegrees(thetaDeg)*cosDegrees(phiDeg),
  };
}

vec3 vec3::max(const vec3& a, const vec3& b) {
  return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}

vec3 vec3::min(const vec3& a, const vec3& b) {
  return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}

vec3 operator*(float lhs, const vec3& rhs) {
  return rhs * lhs;
}

/**
 * \brief 
 * \param r 
 * \param thetaDeg [0, 360], rotation about Y axis
 * \param phiDeg  [-90, 90], xz plane is 0, up+, down-
 */
vec3 fromSpherical(float r, float thetaDeg, float phiDeg) {
  
  return {
    r*cosDegrees(phiDeg)*cosDegrees(thetaDeg),
    r*sinDegrees(phiDeg),
    r*cosDegrees(phiDeg)*sinDegrees(thetaDeg),
  };
}
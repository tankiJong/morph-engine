#include "vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

vec3::vec3(float x, float y, float z)
  : x(x)
  , y(y)
  , z(z) {}

vec3::vec3(const vec2& vec)
  : x(vec.x)
  , y(vec.y)
  , z(0) {}

vec3::vec3(std::initializer_list<float> list) {
  auto begin = list.begin();
  x = *begin++;
  y = *begin;
}
vec3::vec3(const vec3& vec) 
  : x(vec.x)
  , y(vec.y)
  , z(vec.z) {}
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
const vec3 vec3::operator*(float uniformScale) const {
  return { x * uniformScale, y * uniformScale, z * uniformScale };
}
const vec3 vec3::operator/(float inverseScale) const {
  float uniformScale = 1.f / inverseScale;
  return *this*uniformScale;
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
float vec3::magnitude() const {
  return sqrt(x*x + y*y + z*z);
}
float vec3::magnitudeSquared() const {
  return x*x + y*y + z*z;
}
float vec3::normalizeAndGetMagnitude() {
  float len = magnitude();
  float revLen = 1.f / len;
  x *= revLen;
  y *= revLen;
  z *= revLen;
  return len;
}
vec3 vec3::normalized() const {
  float reverseLen = 1.f / magnitude();
  return { x * reverseLen, y * reverseLen, z * reverseLen };
}
float vec3::dot(const vec3& another) const {
  return dot(*this, another);
}
float vec3::dot(const vec3& a, const vec3& b) {
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

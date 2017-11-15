#include "Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Vector3::Vector3(float x, float y, float z)
  : x(x)
  , y(y)
  , z(z) {}

Vector3::Vector3(const Vector2& vec)
  : x(vec.x)
  , y(vec.y)
  , z(0) {}

Vector3::Vector3(std::initializer_list<float> list) {
  auto begin = list.begin();
  x = *begin++;
  y = *begin;
}
Vector3::Vector3(const Vector3& vec) 
  : x(vec.x)
  , y(vec.y)
  , z(vec.z) {}
Vector3::Vector3(const char* str) {
  fromString(str);
}
void Vector3::fromString(const char* data) {
  auto raw = split(data, " ,");
  GUARANTEE_OR_DIE(raw.size() == 2, "illegal input string to parse");

  x = parse<float>(raw[0]);
  y = parse<float>(raw[1]);
}
std::string Vector3::toString() const {
  return Stringf("%f,%f,%f", x, y, z);
}

const Vector3 Vector3::operator+(const Vector3& vecToAdd) const {
  return { vecToAdd.x + x, vecToAdd.y + y, vecToAdd.z + z };
}
const Vector3 Vector3::operator-(const Vector3& vecToSubtract) const {
  return { x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z };
}
const Vector3 Vector3::operator*(float uniformScale) const {
  return { x * uniformScale, y * uniformScale, z * uniformScale };
}
const Vector3 Vector3::operator/(float inverseScale) const {
  float uniformScale = 1.f / inverseScale;
  return *this*uniformScale;
}
void Vector3::operator+=(const Vector3& vecToAdd) {
  x += vecToAdd.x; 
  y += vecToAdd.y; 
  z += vecToAdd.z;
}
void Vector3::operator-=(const Vector3& vecToSubtract) {
  x -= vecToSubtract.x;
  y -= vecToSubtract.y;
  z -= vecToSubtract.z;
}
void Vector3::operator*=(const float uniformScale) {
  x *= uniformScale;
  y *= uniformScale;
  z *= uniformScale;
}
void Vector3::operator/=(const float uniformDivisor) {
  float scale = 1.f / uniformDivisor;
  x *= scale;
  y *= scale;
  z *= scale;
}
void Vector3::operator=(const Vector3& copyFrom) {
  x = copyFrom.x;
  y = copyFrom.y;
  z = copyFrom.z;
}

bool Vector3::operator==(const Vector3& compare) const {
  return x == compare.x && y == compare.y && z == compare.z;
}
bool Vector3::operator!=(const Vector3& compare) const {
  return x != compare.x || y != compare.y || z != compare.z;
}
float Vector3::magnitude() const {
  return sqrt(x*x + y*y + z*z);
}
float Vector3::magnitudeSquared() const {
  return x*x + y*y + z*z;
}
float Vector3::normalizeAndGetMagnitude() {
  float len = magnitude();
  float revLen = 1.f / len;
  x *= revLen;
  y *= revLen;
  z *= revLen;
  return len;
}
Vector3 Vector3::normalized() const {
  float reverseLen = 1.f / magnitude();
  return { x * reverseLen, y * reverseLen, z * reverseLen };
}
float Vector3::dot(const Vector3& another) const {
  return dot(*this, another);
}
float Vector3::dot(const Vector3& a, const Vector3& b) {
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

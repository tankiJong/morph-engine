#include "vec4.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
const vec4 vec4::right  (1, 0, 0, 0);
const vec4 vec4::up     (0, 1, 0, 0);
const vec4 vec4::forward(0, 0, 1, 0);
const vec4 vec4::zero   (0, 0, 0, 0);

vec3 vec4::xyz() const {
  return { x,y,z };
}

vec2 vec4::xy() const {
  return { x,y };
}

vec4::vec4(const vec3& copy) 
  : x(copy.x)
  , y(copy.y)
  , z(copy.z) 
  , w(0) {}

vec4::vec4(const vec3& xyz, float w)
  : x(xyz.x)
  , y(xyz.y)
  , z(xyz.z)
  , w(w) {}

vec4::vec4(const vec2& xy, float z, float w)
  : x(xy.x)
  , y(xy.y)
  , z(z)
  , w(w) {}

void vec4::fromString(const char* text) {
  auto raw = split(text, " ,");
  GUARANTEE_OR_DIE(raw.size() == 4, "illegal input string to parse");

  x = parse<float>(raw[0]);
  y = parse<float>(raw[1]);
  z = parse<float>(raw[2]);
  w = parse<float>(raw[3]);
}

bool vec4::operator==(const vec4& rhs) const {
  return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

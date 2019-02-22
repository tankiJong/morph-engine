#pragma once
#include "Engine/Core/common.hpp"
#include "uvec2.hpp"
#include "Engine/Math/Primitives/vec3.hpp"

class vec3;
class ivec2;
class uvec3;
class ivec3 {
public:
  int x = 0, y = 0, z = 0;
  ivec3() = default;
  ivec3(int x, int y, int z);
  ivec3(const vec3& copy): x((int)copy.x), y((int)copy.y), z((int)copy.z) {};
  ivec3(ivec2 xy, int z);
  explicit ivec3(const uvec3& from);
  ivec2 xy() const;

  ivec3 operator+(const ivec3& rhs) { return { x+rhs.x, y+rhs.y, z+rhs.z }; }
  ivec3& operator+=(const ivec3& rhs) { x+=rhs.x; y+=rhs.y; z+=rhs.z; return *this; }

  ivec3 operator-(const ivec3& rhs) { return { x-rhs.x, y-rhs.y, z-rhs.z }; }
  ivec3& operator-=(const ivec3& rhs) { x-=rhs.x; y-=rhs.y; z-=rhs.z; return *this; }

  ivec3 operator*(const ivec3& rhs) { return { x*rhs.x, y*rhs.y, z*rhs.z }; }
  ivec3& operator*=(const ivec3& rhs) { x*=rhs.x; y*=rhs.y; z*=rhs.z; return *this; }

  ivec3 operator/(const ivec3& rhs) { return { x/rhs.x, y/rhs.y, z/rhs.z }; }
  ivec3& operator/=(const ivec3& rhs) { x/=rhs.x; y/=rhs.y; z/=rhs.z; return *this; }
};

#pragma once
#include "Engine/Core/common.hpp"
#include "uvec2.hpp"

class ivec2;
class uvec3;
class ivec3 {
public:
  int x = 0, y = 0, z = 0;
  ivec3() = default;
  ivec3(int x, int y, int z);
  ivec3(ivec2 xy, int z);
  explicit ivec3(const uvec3& from);
  ivec2 xy() const;
};

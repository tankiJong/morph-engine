#pragma once
#include "Engine/Core/common.hpp"

class ivec3 {
public:
  int x = 0, y = 0, z = 0;
  ivec3() = default;
  ivec3(float x, float y, float z);
};

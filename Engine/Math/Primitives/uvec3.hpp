#pragma once
#include "Engine/Core/common.hpp"
#include "Uvec2.hpp"

class uvec3 {
public:
  uint32_t x = 0, y = 0, z = 0;

  uvec3() = default;
  uvec3(uint x, uint y, uint z);
  uvec3(uvec2 xy, uint z);
  uvec2 xy() const;
};

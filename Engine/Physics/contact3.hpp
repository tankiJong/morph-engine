#pragma once
#include "Engine/Math/Primitives/vec3.hpp"

struct contact3 {
  vec3 position;
  vec3 normal = vec3::zero;

  inline bool valid() const { return normal != vec3::zero; }
};

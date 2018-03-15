#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"

class Texture;

class Sprite {
public:
  Texture* texture = nullptr;
  aabb2    uv;
  vec2     dimension;
  vec2     pivot = vec2(.5f, .5f);
  
  aabb2 bounds() const;
};

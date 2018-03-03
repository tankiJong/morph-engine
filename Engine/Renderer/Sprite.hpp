#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"

class Texture;

class Sprite {
public:
protected:
  Texture* mTexture = nullptr;
  aabb2    mUV;
  vec2     mDimension;
  vec2     mPivot = vec2(.5f, .5f);
  
  aabb2 bounds() const;
};

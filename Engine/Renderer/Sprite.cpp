#include "Sprite.hpp"
aabb2 Sprite::bounds() const {
  vec2 bottomLeft = mDimension * mPivot;

  return { bottomLeft, bottomLeft + mDimension };
}

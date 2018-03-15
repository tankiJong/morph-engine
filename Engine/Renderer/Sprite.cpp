#include "Sprite.hpp"
aabb2 Sprite::bounds() const {
  vec2 bottomleft = - dimension * pivot;

  return { bottomleft, bottomleft + dimension };
}

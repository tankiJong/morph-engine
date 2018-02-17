#include "uvec3.hpp"
uvec3::uvec3(uint x, uint y, uint z)
  : x(x), y(y), z(z) {}

uvec3::uvec3(uvec2 xy, uint z) 
  : x(xy.x), y(xy.y), z(z) {}

uvec2 uvec3::xy() const {
  return { x,y };
}

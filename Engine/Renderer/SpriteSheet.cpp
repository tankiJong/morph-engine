#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Ivec2.hpp"

SpriteSheet::SpriteSheet(Texture& texture
                         , int tilesWidth, int tilesHeight)
  : m_spriteSheetTexture(texture)
    , m_spriteLayout(tilesWidth, tilesHeight) {
  
}

aabb2 SpriteSheet::getTexCoords(const ivec2& spriteCoords) const {
  float unitSzieX = 1.f / m_spriteLayout.x;
  float unitSizeY = 1.f / m_spriteLayout.y;
  vec2 mins = vec2(
    unitSzieX * (float) spriteCoords.x, 
    unitSizeY * (float) spriteCoords.y);
  vec2 maxs = vec2(
    unitSzieX * (float) (spriteCoords.x + 1), 
    unitSizeY * (float) (spriteCoords.y + 1));

  return aabb2(mins, maxs);
}

aabb2 SpriteSheet::getTexCoordsByIndex(int spriteIndex) const {
  int x = spriteIndex % m_spriteLayout.x;
  int y = (spriteIndex - x) / m_spriteLayout.x;

  return getTexCoords(ivec2(x, y));
}

const Texture& SpriteSheet::getTexture() const {
  return m_spriteSheetTexture;
}

int SpriteSheet::getNumSprites() const {
  return m_spriteLayout.x * m_spriteLayout.y;
}

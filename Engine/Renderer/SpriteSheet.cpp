#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"

SpriteSheet::SpriteSheet(Texture& texture
                         , int tilesWidth, int tilesHeight)
           : m_spriteSheetTexture(texture)
           , m_spriteLayout(tilesWidth, tilesHeight) {

}

AABB2 SpriteSheet::getTexCoords(const IntVector2& spriteCoords) const {
  float unitSzieX = 1.f / m_spriteLayout.x;
  float unitSizeY = 1.f / m_spriteLayout.y;
  Vector2 mins = Vector2(unitSzieX * (float)spriteCoords.x, unitSizeY * (float)(spriteCoords.y + 1));
  Vector2 maxs = Vector2(unitSzieX * (float)(spriteCoords.x + 1), unitSizeY * (float)spriteCoords.y);

  return AABB2(mins, maxs);
}

AABB2 SpriteSheet::getTexCoordsByIndex(int spriteIndex) const {
  int x = spriteIndex % m_spriteLayout.x;
  int y = (spriteIndex - x) / m_spriteLayout.x;

  return getTexCoords(IntVector2(x, y));
}

Texture* SpriteSheet::getTexture() const {
  return &m_spriteSheetTexture;
}

int SpriteSheet::getNumSprites() const {
  return m_spriteLayout.x * m_spriteLayout.y;
}

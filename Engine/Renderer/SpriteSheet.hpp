#pragma once
#include "Engine/Math/IntVector2.hpp"

class AABB2;
class Texture;
class IntVector2;

class SpriteSheet {
public:
  SpriteSheet(Texture& texture, int tilesWide, int tilesHigh);
  ~SpriteSheet() {}

  AABB2 getTexCoords(const IntVector2& spriteCoords) const; // for sprites
  AABB2 getTexCoordsByIndex(int spriteIndex) const; // for sprite animations
  const Texture& getTexture() const;
  int getNumSprites() const;

private:
  Texture& m_spriteSheetTexture; // Texture w/grid-based layout of sprites
  IntVector2 m_spriteLayout; // # of sprites across, and down, on the sheet
};

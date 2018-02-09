#pragma once
#include "Engine/Math/IntVec2.hpp"

class aabb2;
class Texture;
class IntVec2;

class SpriteSheet {
public:
  SpriteSheet(Texture& texture, int tilesWide, int tilesHigh);
  ~SpriteSheet() {}

  aabb2 getTexCoords(const IntVec2& spriteCoords) const; // for sprites
  aabb2 getTexCoordsByIndex(int spriteIndex) const; // for sprite animations
  inline IntVec2 layout() const { return  m_spriteLayout; }
  const Texture& getTexture() const;
  int getNumSprites() const;

private:
  Texture& m_spriteSheetTexture; // Texture w/grid-based layout of sprites
  IntVec2 m_spriteLayout; // # of sprites across, and down, on the sheet
};

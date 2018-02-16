#pragma once
#include "Engine/Math/Ivec2.hpp"

class aabb2;
class Texture;
class ivec2;

class SpriteSheet {
public:
  SpriteSheet(Texture& texture, int tilesWide, int tilesHigh);
  ~SpriteSheet() {}

  aabb2 getTexCoords(const ivec2& spriteCoords) const; // for sprites
  aabb2 getTexCoordsByIndex(int spriteIndex) const; // for sprite animations
  inline ivec2 layout() const { return  m_spriteLayout; }
  const Texture& getTexture() const;
  int getNumSprites() const;

private:
  Texture& m_spriteSheetTexture; // Texture w/grid-based layout of sprites
  ivec2 m_spriteLayout; // # of sprites across, and down, on the sheet
};

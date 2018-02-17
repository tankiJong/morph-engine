#pragma once
#include "Engine/Math/Primitives/uvec2.hpp"

class aabb2;
class Texture;
class uvec2;

class SpriteSheet {
public:
  SpriteSheet(Texture& texture, int tilesWide, int tilesHigh);
  ~SpriteSheet() {}

  aabb2 getTexCoords(const uvec2& spriteCoords) const; // for sprites
  aabb2 getTexCoordsByIndex(int spriteIndex) const; // for sprite animations
  inline uvec2 layout() const { return  m_spriteLayout; }
  const Texture& getTexture() const;
  int getNumSprites() const;

private:
  Texture& m_spriteSheetTexture; // Texture w/grid-based layout of sprites
  uvec2 m_spriteLayout; // # of sprites across, and down, on the sheet
};

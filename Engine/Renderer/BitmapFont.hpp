﻿#pragma once
#include <string>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

class AABB2;
class BitmapFont {
  friend class Renderer;

public:
  const std::string m_fontName;
  
  AABB2 GetUVsForGlyph(int glyphUnicode) const; // pass ‘A’ or 65 for A, etc.
  float GetStringWidth(const std::string& asciiText, float cellHeight, float aspectScale) const;
  float GetGlyphAspect(int glyphUnicode) const { UNUSED(glyphUnicode);  return m_baseAspect; } // will change later

private:
  explicit BitmapFont(const std::string& fontName, const SpriteSheet& glyphSheet,
                      float baseAspect = 1.f); // private, can only called by Renderer (friend class)
  ~BitmapFont();
private:
  const SpriteSheet&	m_spriteSheet; // used internally; assumed to be a 16x16 glyph sheet
  float	m_baseAspect = 1.0f; // used as the base aspect ratio for all glyphs

};

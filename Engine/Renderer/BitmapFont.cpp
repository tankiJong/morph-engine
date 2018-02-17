#include "BitmapFont.hpp"

#include "Engine/Math/Primitives/AABB2.hpp"

BitmapFont* gDefaultFont = nullptr;
aabb2 BitmapFont::getUVsForGlyph(int glyphUnicode) const {
  return m_spriteSheet.getTexCoordsByIndex(glyphUnicode);
}
float BitmapFont::getStringWidth(const std::string& asciiText, float cellHeight, float aspectScale) const {
  return float(asciiText.length()) * cellHeight * aspectScale;
}

float BitmapFont::getCharWidth(float cellHeight, char c, float aspectScale) {
  UNUSED(c);
  return cellHeight * aspectScale;
}

int BitmapFont::maxCharacterInWidth(float spaceWidth, float cellHeight, float aspectScale) const {
  float charWidth = cellHeight * aspectScale;
  return int(spaceWidth / charWidth);
}

void BitmapFont::setDefaultFont(BitmapFont& font) {
  gDefaultFont = &font;
}
BitmapFont* BitmapFont::getDefaultFont() {
  return gDefaultFont;
}

BitmapFont::BitmapFont(const std::string& fontName, const SpriteSheet& glyphSheet, float baseAspect)
  : m_fontName(fontName)
  , m_spriteSheet(glyphSheet)
  , m_baseAspect(baseAspect) {
}

BitmapFont::~BitmapFont() {
  delete &m_spriteSheet;
}

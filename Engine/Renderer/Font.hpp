#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include <vector>
#include "Engine/File/Path.hpp"
#include "Engine/Persistence/json.forward.hpp"

class Texture;

class  Glyph {
public:
  Glyph() = default;
  Glyph(uint id, const vec2& offset, const aabb2& uv, float width, float height);
  uint texIndex;
  uint id = 0;
  vec2 offset;
  aabb2 uv;

  inline float width(float size) const { return mDimension.x * size; }
  inline float height(float size) const { return mDimension.y * size; }
  inline vec2 size(float _size) const { return mDimension * _size; }
  inline vec2& dimension() { return mDimension; }
protected:
  vec2 mDimension;
};

class Font {
  friend Font* fromJson(const fs::path& path);
  class Face {
  public:
    Face() = default;
    Face(uint code, int glyphIndex, float advance);
    uint code = 0;
    int glyph = 0;
    inline float advance(float size) const { return size * mAdvance; }
  protected:
    float mAdvance = 0;
  };
public:
  Font(std::string name, span<const Texture*> textures, std::vector<Glyph>&& glyphs);
  Font(std::string name, span<const Texture*> textures, span<Glyph> glyphs);
  Font() = default;

  void addFace(uint code, int glyphIndex, float advance);
  inline float lineHeight(float size) const { return mLineHeight * size; }
  inline float ascender(float size) const { return mAscender * size; }
  inline float descender(float size) const { return mDescender * size; }

  static constexpr uint NUM_MAX_TEXTURE_SUPPORT = 10u;
protected:
  std::array<const Texture*, NUM_MAX_TEXTURE_SUPPORT> mTextures;
  std::vector<Glyph> mGlyphs;
  std::vector<Face> mFaces;
  std::string mName;
  Glyph mDefault;
  float mLineHeight = 0.f;
  float mAscender = 0.f;
  float mDescender = 0.f;
};

Font* fromJson(const fs::path& path);



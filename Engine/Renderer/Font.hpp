#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include <vector>
#include "Engine/File/Path.hpp"
#include "Engine/Persistence/json.forward.hpp"

class Texture;
class Font;
class  Glyph {
  friend void from_json(const json& j, Glyph& g);
  friend Font* fromJson(const fs::path& path);
public:
  Glyph() = default;
  Glyph(uint id, const vec2& offset, const aabb2& uv, float w, float h);
  uint texIndex = 0;
  uint id = 0;
  aabb2 uv;

  inline float width(float  size)  const { return mDimension.x * size;  }
  inline float height(float size)  const { return mDimension.y * size;  }
  inline vec2  size(float   _size) const { return mDimension   * _size; }
  inline vec2 offset(float size) const { return mOffset * size; }
  inline vec2& dimension() { return mDimension; }

protected:
  vec2 mOffset;
  vec2 mDimension;
};

class Font {
  friend Font* fromJson(const fs::path& path);


  class Face {
    friend Font* fromJson(const fs::path& path);

    class Kerning {
    public:
      Kerning(uint previous, float offset);
      uint previousCode = 0;
      inline float offeset(float size) const { return mOffset * size; }
    protected:
      float mOffset = 0;
    };
  public:
    Face() = default;
    Face(uint code, const Glyph* glyph, float advance);
    aabb2 uv() const;
    uint code = 0;
    inline vec2 size(float size) const { return mGlyph ? mGlyph->size(size) : vec2::zero; };
    inline float advance(float size) const { return size * mAdvance; }
    float kerning(uint previous, float size) const;
    vec2 offset(float size) const;
  protected:
    float mAdvance = 0;
    const Glyph* mGlyph = nullptr;
    std::vector<Kerning> mKernings;
  };

public:
  static constexpr uint NUM_MAX_TEXTURE_SUPPORT = 10u;
  
  Font(std::string name, span<const Texture*> textures, std::vector<Glyph>&& glyphs);
  Font(std::string name, span<const Texture*> textures, span<Glyph> glyphs);
  Font() = default;

  void addFace(uint code, uint glyphIndex, float advance);
  void addFace(uint code, float advance);
  inline float lineHeight(float size) const { return mLineHeight * size; }
  inline float ascender(float size) const { return mAscender * size; }
  inline float descender(float size) const { return mDescender * size; }


  // figure out how to move cursor from *previous* to c
  float advance(char previous, char c, float size, float aspectScale = 1.f);
  float advance(std::string_view text, float size, float aspectScale = 1.f);
  
  // how big is the text, http://www.angelcode.com/products/bmfont/doc/render_text.html
  aabb2 bounds(char c, float size, float aspectScale = 1.f) const;

  aabb2 uv(char c);
protected:
  std::string mName;
  std::vector<Glyph> mGlyphs;
  std::vector<Face> mFaces;
  std::vector<const Texture*> mTextures;
  float mLineHeight = 0.f;
  float mAscender = 0.f;
  float mDescender = 0.f;
};

Font* fromJson(const fs::path& path);



#include "Font.hpp"
#include <utility>
#include "Engine/Persistence/json.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Texture.hpp"

S<const Font> Font::sDefaultFont = S<const Font>{};

Glyph::Glyph(uint id, const vec2& offset, const aabb2& uv, float w, float h)
  : id(id)
  , uv(uv)
  , mOffset(offset)
  , mDimension(w, h) {}

Font::Face::Face(uint code, const Glyph* glyph, float advance)
  : code(code)
  , mAdvance(advance)
  , mGlyph(glyph) {}

aabb2 Font::Face::uv() const {
  return mGlyph ? mGlyph->uv : aabb2();
}

float Font::Face::kerning(uint previous, float size) const {
  if (previous == 0u || mKernings.empty()) return 0.f;
  
  uint start = 0u;
  uint end = (uint)mKernings.size() - 1u;

  uint i = (end - start) / 2u;

  while (mKernings[i].previousCode != previous) {
    if(mKernings[i].previousCode < previous) {
      start = i;
    } else {
      end = i;
    }

    const uint ii = (end - start) / 2u;
    if (i == ii) break;

    if(ii == i + 1u || ii == i - 1u) {
      if (mKernings[ii].previousCode == previous) i = ii;
      break;
    }
    i = ii;
  }

  return mKernings[i].previousCode == previous ? mKernings[i].offeset(size) : 0.f;
}

vec2 Font::Face::offset(float size) const {
  return mGlyph ? mGlyph->offset(size) : vec2::zero;
}

Font::Face::Kerning::Kerning(uint previous, float offset): previousCode(previous), mOffset(offset) {}

Font::Font(std::string name, span<const Texture*> textures, std::vector<Glyph>&& glyphs)
  : name(std::move(name))
  , mGlyphs(glyphs) {
  for(int i = 0; i < textures.size(); i++) {
    mTextures[i] = textures[i];
  }
}

void Font::addFace(uint code, uint glyphIndex, float advance) {
  if (mFaces.size() <= code) mFaces.resize(code + 1);

  mFaces[code] = Face( code, &mGlyphs.at(glyphIndex), advance);
}

void Font::addFace(uint code, float advance) {
  if (mFaces.size() <= code) mFaces.resize(code + 1);

  mFaces[code] = Face(code, nullptr, advance);
}

float Font::advance(std::string_view text, float size, float aspectScale) const {
  // move forward one by one
  if (text.empty()) return 0;
  auto it = text.begin();
  float totalAd = advance(0, *it, size, aspectScale);
  while(it != text.end() - 1) {
    float ad = advance(*it, *(++it), size, aspectScale);
    totalAd += ad;
  }

  return totalAd;
}

float Font::advance(char previous, char c, float size, float aspectScale) const {
  const Face& f = mFaces[c];
  float advance = f.advance(size);
  float kerning = f.kerning(previous, size);
  return (advance + kerning) * aspectScale;
}

/**
 * \brief the relative box related to the base point
 */
aabb2 Font::bounds(char c, float size, float aspectScale) const {
  const Face& f = mFaces[c];

  vec2 xyOffset = f.offset(size);
  vec2 mins(xyOffset.x, xyOffset.y);
  vec2 maxs = mins + f.size(size);

  maxs.x = mins.x + (maxs.x - mins.x) * aspectScale;
  
  return { mins, maxs };

}

aabb2 Font::uv(char c) const {
  return mFaces[c].uv();
}

void from_json(const json& j, Glyph& g) {
  g.texIndex = j.at("texture");
  g.mOffset = vec2(j.at("offset_x").get<float>(), j.at("offset_y").get<float>());
  g.dimension().x = j.at("width");
  g.dimension().y = j.at("height");

  vec2 bottomLeft(j.at("tex_x").get<float>(), j.at("tex_y").get<float>());
  vec2 size(j.at("width").get<float>(), j.at("height").get<float>());

  g.uv.mins = bottomLeft;
  g.uv.maxs = g.uv.mins + size;
}

Font* fromJson(const fs::path& path) {
  FileSystem& fsys = FileSystem::Get();
  
  std::optional<Blob> blob = fsys.asBuffer(path);

  if(!blob) return nullptr;

  std::string_view str(blob->as<char*>());

  json meta = json::parse(str.begin(), str.end());

  auto glyphsNode = meta.at("glyphs");
  EXPECTS(glyphsNode.is_array());

  float size = meta.at("size");
  std::vector<Glyph> glyphs;
  glyphs.reserve(glyphsNode.size());
  for(auto& glyph: glyphsNode) {
    glyphs.push_back(glyph.get<Glyph>());
    Glyph& g = glyphs.back();
    g.id = (uint)glyphs.size() - 1u;
    g.mOffset /= size;
    g.dimension() /= size;

//    vec2 gsize = g.uv.size() / size;
    g.uv.mins /= 1024.f;
    g.uv.maxs /= 1024.f;

  }

  Font* font = new Font(meta.at("name").get<std::string>(), {}, std::move(glyphs));
  TODO("set default(falloff)");
  font->mLineHeight = meta.at("line_height").get<float>() / size;
  font->mAscender = meta.at("ascender").get<float>() / size;
  font->mDescender = meta.at("descender").get<float>() / size;

  auto& textures = meta.at("images");
  EXPECTS(textures.is_array());
  for(auto& texpath: textures) {
    std::string p = texpath;

    auto tex = Resource<Texture>::get(p);
    if(!tex) {
      Resource<Texture>::define(p);
      tex = Resource<Texture>::get(p);
    }

    font->mTextures.push_back(tex.get());
  }

  // the code expects the element at the end is with the biggest index
  auto& faces = meta.at("faces");
  EXPECTS(faces.is_object());

  for (auto kv = faces.rbegin(); kv != faces.rend(); ++kv) {
    uint code = parse<uint>(kv.key());
    float advance = kv.value().at("advance");

    auto g = kv.value().find("glyph");
    if(g != kv.value().end()) {
      uint index = *g;
      font->addFace(code, index, advance / size);
    } else {
      font->addFace(code, advance /size);
    }
  }

  auto& kernings = meta.at("kernings");
  EXPECTS(kernings.is_array());

  if (kernings.empty()) return font;

  for(auto k = kernings.begin(); k != kernings.end(); ++k) {
    uint first = k->at("first");
    uint second = k->at("second");
    float offset = k->at("offset");
    offset /= size;

    font->mFaces[second].mKernings.emplace_back(first, offset);
  }

  for(auto& face: font->mFaces) {
    std::sort(face.mKernings.begin(), face.mKernings.end(), [](Font::Face::Kerning& a, Font::Face::Kerning& b) {
      return a.previousCode < b.previousCode;
    });
  }

  return font;
}

ResDef<Font> Resource<Font>::load(const std::string& file) {
  Font* font = fromJson(file);

  return { font->name, font };
}

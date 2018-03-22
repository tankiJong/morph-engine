#include "Font.hpp"
#include "Engine/Persistence/json.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

Glyph::Glyph(uint id, const vec2& offset, const aabb2& uv, float w, float h)
  : id(id)
  , offset(offset)
  , uv(uv)
  , mDimension(w, h) {}

Font::Face::Face(uint code, int glyphIndex, float advance)
  : code(code)
  , glyph(glyphIndex)
  , mAdvance(advance) {}

void Font::addFace(uint code, int glyphIndex, float advance) {
  if (mFaces.size() <= code) mFaces.resize(code + 1);

  mFaces.emplace(mFaces.begin() + code, code, glyphIndex, advance);
}


void from_json(const json& j, Glyph& g) {
  g.texIndex = j.at("texture");
  g.offset = vec2(j.at("offset_x").get<float>(), j.at("offset_y").get<float>());
  g.dimension().x = j.at("width");
  g.dimension().y = j.at("height");

  vec2 mins(j.at("uv_x").get<float>(), j.at("uv_y").get<float>());
  vec2 maxs(j.at("uv_width").get<float>(), j.at("uv_height").get<float>());
}

Font* fromJson(const fs::path& path) {
  FileSystem& fsys = FileSystem::Get();
  
  std::optional<Blob> blob = fsys.asBuffer(path);

  if(!blob) return nullptr;

  std::string_view str(blob->as<char*>());

  json meta = json::parse(str.begin(), str.end());


  UNIMPLEMENTED("create texture from resource");

  auto glyphsNode = meta.at("glyphs");
  EXPECTS(glyphsNode.is_array());

  float size = meta.at("size");
  std::vector<Glyph> glyphs(glyphsNode.size());
  for(auto& glyph: glyphsNode) {
    glyphs.push_back(glyph.get<Glyph>());
    Glyph& g = glyphs.back();
    g.offset /= size;
    g.dimension() /= size;

  }

  Font* font = new Font(meta.at("name").get<std::string>(), {}, std::move(glyphs));
  UNIMPLEMENTED("set default(falloff)");
  font->mLineHeight = meta.at("line_height").get<float>() / size;
  font->mAscender = meta.at("ascender").get<float>() / size;
  font->mDescender = meta.at("descender").get<float>() / size;

  // the code expects the element at the end is with the biggest index
  auto& faces = meta.at("faces");
  EXPECTS(faces.is_object());
  for (json::reverse_iterator kv = faces.rbegin(); kv < faces.rend(); ++kv) {
    uint code = parse<uint>(kv.key());
    float advance = kv.value().at("advance");
    uint index = kv.value().at("glyph");
    font->addFace(code, index, advance);
  }

}

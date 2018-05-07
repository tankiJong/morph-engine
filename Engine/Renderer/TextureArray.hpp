#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/uvec3.hpp"
#include "Engine/Renderer/Texture.hpp"

class Texture;
class TextureArray {
public:
  explicit TextureArray(uvec2 dimen, uint layerCount, eTextureFormat format = TEXTURE_FORMAT_RGBA8);
  explicit TextureArray(uint width, uint height, uint layerCount, eTextureFormat format = TEXTURE_FORMAT_RGBA8);
  void setLayer(Texture* tex, uint layer);

  inline uint handle() const { return mHandle; }
  inline uvec3 dimension() const { return mDimension; }
protected:
  uint mHandle = 0;
  uint mLayerCount = 0;
  uvec3 mDimension;
  eTextureFormat mFormat;
  void setup();
};

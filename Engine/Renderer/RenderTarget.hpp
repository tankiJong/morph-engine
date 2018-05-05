#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/Texture.hpp"

class RenderTarget: public Texture {
public:
  RenderTarget(uint width, uint height, eTextureFormat format = TEXTURE_FORMAT_RGBA8);
  inline RenderTarget* clone() const { return (RenderTarget*)Texture::clone(); }
protected:
  bool setup(uint width, uint height, eTextureFormat format);
};

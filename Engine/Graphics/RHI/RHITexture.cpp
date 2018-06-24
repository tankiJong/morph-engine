#include "RHITexture.hpp"

RHITexture::sptr_t RHITexture::create2D(uint width, uint height, const void* data, size_t size, BindingFlag flag) {
  flag |= BindingFlag::RenderTarget;

  sptr_t tex = sptr_t(new RHITexture(width, height, flag));
  return tex->rhiInit(data, size) ? tex : nullptr;
}


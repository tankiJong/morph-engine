#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Renderer/Texture.hpp"

class Sampler {
public:
  Sampler();
  Sampler(eTextureWrapMode wrapMode, eTextureSampleMode sampleMode = TEXTURE_SAMPLE_LINEAR);
  Sampler(eTextureSampleMode sampleMode, eTextureWrapMode wrapMode = TEXTURE_WRAP_REPEAT);
  ~Sampler();

  bool create(eTextureSampleMode sampleMode = TEXTURE_SAMPLE_LINEAR, eTextureWrapMode wrapMode = TEXTURE_WRAP_REPEAT);
  void destroy();

  inline uint handle() const { return mSamplerHandle; }

  static const Sampler& Default();
  static const Sampler& Linear();
  static const Sampler& Point();
protected:
  uint mSamplerHandle;
};

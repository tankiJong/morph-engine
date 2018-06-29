#include "Sampler.hpp"
#include "External/gl/glcorearb.h"
#include "glFunctions.hpp"

Sampler::Sampler()
  : mSamplerHandle(NULL) {
  create();
}

Sampler::Sampler(eTextureWrapMode wrapMode, eTextureSampleMode sampleMode)
  : mSamplerHandle(NULL) {
  create(sampleMode, wrapMode);
}

Sampler::Sampler(eTextureSampleMode sampleMode, eTextureWrapMode wrapMode)
  : mSamplerHandle(NULL) {
  create(sampleMode, wrapMode);
}

Sampler::~Sampler() {
  destroy();
}

bool Sampler::create(eTextureSampleMode sampleMode, eTextureWrapMode wrapMode) {
  // create the sampler handle if needed; 
  if (mSamplerHandle == NULL) {
    glGenSamplers(1, &mSamplerHandle);
    if (mSamplerHandle == NULL) {
      return false;
    }
  }

  // setup wrapping
  glSamplerParameteri(mSamplerHandle, GL_TEXTURE_WRAP_S, toGLType(wrapMode));
  glSamplerParameteri(mSamplerHandle, GL_TEXTURE_WRAP_T, toGLType(wrapMode));
  glSamplerParameteri(mSamplerHandle, GL_TEXTURE_WRAP_R, toGLType(wrapMode));

  // filtering; 

  switch(sampleMode) { 
    case TEXTURE_SAMPLE_POINT:
      glSamplerParameteri(mSamplerHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    break;
    case TEXTURE_SAMPLE_LINEAR:
      glSamplerParameteri(mSamplerHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    break;
    default: ;
  }
  glSamplerParameteri(mSamplerHandle, GL_TEXTURE_MAG_FILTER, toGLType(sampleMode));

  return true;
}

void Sampler::destroy() {
  if (mSamplerHandle != NULL) {
    glDeleteSamplers(1, &mSamplerHandle);
    mSamplerHandle = NULL;
  }
}

Sampler* gPointSampler;
Sampler* gLinearSampler;

const Sampler& Sampler::Default() {
  if(!gPointSampler) {
    gPointSampler = new Sampler();
  }
  return *gPointSampler;
}

const Sampler& Sampler::Linear() {
  if (!gLinearSampler) {
    gLinearSampler = new Sampler(TEXTURE_SAMPLE_LINEAR);
  }
  return *gLinearSampler;
}

const Sampler& Sampler::Point() {
  if (!gPointSampler) {
    gPointSampler = new Sampler();
  }
  return *gPointSampler;
}

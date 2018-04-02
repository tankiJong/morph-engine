#include "Sampler.hpp"
#include "External/gl/glcorearb.h"
#include "glFunctions.hpp"

Sampler::Sampler()
  : mSamplerHandle(NULL) {
  create();
}

Sampler::~Sampler() {
  destroy();
}

bool Sampler::create() {
  // create the sampler handle if needed; 
  if (mSamplerHandle == NULL) {
    glGenSamplers(1, &mSamplerHandle);
    if (mSamplerHandle == NULL) {
      return false;
    }
  }

  // setup wrapping
  glSamplerParameteri(mSamplerHandle, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glSamplerParameteri(mSamplerHandle, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glSamplerParameteri(mSamplerHandle, GL_TEXTURE_WRAP_R, GL_REPEAT);

  // filtering; 
  glSamplerParameteri(mSamplerHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glSamplerParameteri(mSamplerHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return true;
}

void Sampler::destroy() {
  if (mSamplerHandle != NULL) {
    glDeleteSamplers(1, &mSamplerHandle);
    mSamplerHandle = NULL;
  }
}
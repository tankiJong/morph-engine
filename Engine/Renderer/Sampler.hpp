#pragma once

#include "Engine/Core/common.hpp"
class Sampler {
public:
  Sampler();
  ~Sampler();

  bool create();
  void destroy();

  uint getHandle() const { return mSamplerHandle; }
protected:
  uint mSamplerHandle;
};

#pragma once

#include "Engine/Core/common.hpp"

class Program {
public:
  Program();
  Shader& stage(eShaderType type) {
    return mShaders[type];
  }

  void compile();
protected:
  std::array<Shader, NUM_SHADER_TYPE> mShaders;
};
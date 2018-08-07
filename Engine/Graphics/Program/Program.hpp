#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/Shader.hpp"
class Program {
public:
  using sptr_t = S<Program>;
  Program();
  Shader& stage(eShaderType type) {
    return mShaders[type];
  }

  void compile();
protected:
  std::array<Shader, NUM_SHADER_TYPE> mShaders;
};
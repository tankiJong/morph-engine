#pragma once
#include "Engine/Core/common.hpp"
#include <array>
#include "ShaderStage.hpp"

class Blob;
class ShaderProgram {
  friend class Renderer;
public:
  ~ShaderProgram() = default;
  bool fromFile(const char*  relativePath, const char* defineArgs = nullptr);
  static uint createAndLinkProgram(uint vs, uint fs, uint handle = 0);
  static void logProgramError(uint programId);
  uint programHandle = NULL;
protected:
  std::array<ShaderStage, NUM_SHADER_TYPE> stages;
  ShaderProgram() = default;
};

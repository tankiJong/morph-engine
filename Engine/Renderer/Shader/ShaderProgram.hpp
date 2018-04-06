#pragma once
#include "Engine/Core/common.hpp"
#include <array>
#include "ShaderStage.hpp"

class Blob;
class ShaderProgram {
public:
  ShaderProgram() = default;
  ~ShaderProgram() = default;
  bool fromFile(const char*  relativePath, const char* defineArgs = nullptr);
  inline uint handle() const { return programHandle; }
  static uint createAndLinkProgram(uint vs, uint fs, uint handle = 0);
  static void logProgramError(uint programId);
protected:
  std::array<ShaderStage, NUM_SHADER_TYPE> stages;
  uint programHandle = NULL;
};

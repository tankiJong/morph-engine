#pragma once
#include "Engine/Core/common.hpp"
#include <array>
#include "ShaderStage.hpp"

typedef uint GLuint;
typedef uint GLenum;
class Blob;
class ShaderProgram {
  friend class Renderer;
public:
  ~ShaderProgram() = default;
  bool fromFile(const char*  relativePath, const char* defineArgs = nullptr);
  static GLuint createAndLinkProgram(GLuint vs, GLuint fs, GLuint handle = 0);
  static void logProgramError(GLuint programId);
  GLuint programHandle = NULL;
protected:
  std::array<ShaderStage, NUM_SHADER_TYPE> stages;
  ShaderProgram() = default;
};

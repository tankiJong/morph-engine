#pragma once

typedef unsigned int GLuint;
typedef unsigned int GLenum;
class Blob;
class ShaderProgram {
  friend class Renderer;
public:
  ~ShaderProgram() = default;
  bool fromFile(const char*  relativePath, const char* defineArgs = nullptr);

  static GLuint createAndLinkProgram(GLuint vs, GLuint fs);
  static void logShaderError(GLuint shaderId);
  static GLuint loadShader(const char* shaderStr, GLenum type, const char* defineArgs = nullptr);
  static GLuint loadShader(const Blob& rawShader, GLenum type, const char* defineArgs = nullptr);
  GLuint programHandle = 0;
protected:
  ShaderProgram() = default;
  static Blob injectDefine(const char* buffer, size_t size, const char* defines);
  static Blob injectInclude(const char* buffer, size_t size);
};

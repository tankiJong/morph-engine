#pragma once
#include <string>

struct Attribute {
  friend class ShaderProgram;
  std::string name;
  int count = 0;
  int location = -1;
  unsigned type = ~0u;

};

struct Uniform {
  friend class ShaderProgram;
  std::string name;
};

enum ShaderType {
  SHADER_TYPE_VERTEX,
  SHADER_TYPE_FRAGMENT,
};

class ShaderStage {
public:
  ShaderType type;

};

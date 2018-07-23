#include "Engine/Graphics/RHI/Shader.hpp"
#include "Program.hpp"

Program::Program() {
  for(uint i = 0; i < NUM_SHADER_TYPE; i++) {
    mShaders[i].setType((eShaderType)i);
  }
}

void Program::compile() {
  for(Shader& shader: mShaders) {
    shader.compile();
  }
}

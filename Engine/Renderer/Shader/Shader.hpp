#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/type.h"
#include "ShaderProgram.hpp"

class Shader {
public:
  ShaderProgram* prog;
  render_state state;

  void enableBlending(eBlendOp op, eBlendFactor src, eBlendFactor dst);
  void disableBlending();

  void setDepth(eCompare comp, bool willWrite);
  void disableDepth();
};

Shader* fromYaml(fs::path& file);
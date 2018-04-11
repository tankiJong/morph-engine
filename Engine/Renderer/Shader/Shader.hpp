#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/type.h"
#include "ShaderProgram.hpp"
#include "Engine/Core/Resource.hpp"

class Shader {
public:
  std::string name;
  void enableBlending(eBlendOp op, eBlendFactor src, eBlendFactor dst);
  void disableBlending();

  void setDepth(eCompare comp, bool willWrite);
  void disableDepth();

  inline ShaderProgram*& prog() { return mProg; }
  inline ShaderProgram* prog() const { return mProg; }

  inline render_state& state() { return mState; }
  inline const render_state& state() const { return mState; }
protected:
  ShaderProgram* mProg;
  render_state mState;
};

owner<Shader*> fromYaml(const fs::path& file);

template<>
ResDef<Shader> Resource<Shader>::load(const fs::path& file);
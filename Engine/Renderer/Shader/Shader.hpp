#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/type.h"
#include "ShaderProgram.hpp"
#include "Engine/Core/Resource.hpp"

#define SHADER_LAYER_OPAQUE 0
#define SHADER_LAYER_ALPHA  0x100
class Shader {
public:
  std::string name;

  /* it is opaque(0)/alpha(0x100)?
   * when sorting the renderable, use `layer << 8 | sort` as the order
   */
  uint layer = SHADER_LAYER_OPAQUE;
  uint sort = 0;
  inline uint order() const { return layer << 8 | sort; }
  void enableBlending(eBlendOp op, eBlendFactor src, eBlendFactor dst);
  void disableBlending();

  void setDepth(eCompare comp, bool willWrite);
  void disableDepth();

  inline ShaderProgram*& prog() { return mProg; }
  inline ShaderProgram* prog() const { return mProg; }

  inline render_state& state() { return mState; }
  inline const render_state& state() const { return mState; }
protected:
  ShaderProgram * mProg = nullptr;
  render_state mState;
};

owner<Shader*> fromYaml(const fs::path& file);

template<>
ResDef<Shader> Resource<Shader>::load(const fs::path& file);
#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/type.h"
#include "ShaderProgram.hpp"
#include "Engine/Core/Resource.hpp"
#include "Engine/Persistence/yaml.hpp"
#include "Game/GameCommon.hpp"
#define SHADER_LAYER_OPAQUE 0
#define SHADER_LAYER_ALPHA  0x100
class ShaderPass {
public:

  ShaderPass() = default;
  ShaderPass(ShaderPass&& from);
  ShaderPass(const ShaderPass&) = delete;
  ShaderPass& operator=(const ShaderPass&) = delete;
  ShaderPass& operator=(ShaderPass&& from);
  
  ~ShaderPass();
  
  /* it is opaque(0)/alpha(0x100)?
  * when sorting the renderable, use `layer << 8 | sort` as the order
  */
  uint layer;
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

  inline static uint order(uint layer, uint sort) { return layer << 8 | sort; }

protected:
  owner<ShaderProgram*> mProg = nullptr;
  render_state mState;
};


namespace YAML {
  class Node;
  template<>
  struct convert<ShaderPass> {
    static bool decode(const Node& node, ShaderPass& rhs);;
  };

  template<>
  struct convert<render_state> {
    static bool decode(const Node& node, render_state& rs);;
  };
}
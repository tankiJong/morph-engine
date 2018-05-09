#include "AfterEffect.hpp"
#include "Engine/Renderer/RenderTarget.hpp"
#include "Engine/Renderer/Geometry/Vertex.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/glFunctions.hpp"

AfterEffect::AfterEffect(Renderer& renderer, RenderTarget& target, const Shader& effectShader)
  : mRenderer(renderer)
  , mTarget(target)
  , mEffectShader(&effectShader)
  , mEffectTarget(uint(target.dimension().x), 
                  uint(target.dimension().y), target.format()) {
}

void AfterEffect::apply() {
  preEffect();
  effect();
  postEffect();
}

void AfterEffect::effect() {
  
}

void AfterEffect::applyQuad() {
  static std::array<vertex_pcu_t, 3> verts;
  // mRenderer.drawMeshImmediate(verts.data(), 3, DRAW_TRIANGES);
  glDrawArrays(toGLType(DRAW_TRIANGES), 0, 3);
}

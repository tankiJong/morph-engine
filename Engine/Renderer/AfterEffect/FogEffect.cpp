#include "FogEffect.hpp"
#include "Engine/Renderer/Renderer.hpp"

FogEffect::FogEffect(Renderer& renderer, RenderTarget& target, 
                     const Shader& effectShader, const Texture& depthTex)
  : AfterEffect(renderer, target, effectShader)
  , mDepthTex(depthTex) {}

void FogEffect::effect() {
  mRenderer.setTexture(TEXTURE_DIFFUSE, &mDepthTex);
  mRenderer.setShader(mEffectShader, 0);
  mEffectCamera.setColorTarget(&mTarget, 0);

  mRenderer.setCamera(&mEffectCamera);
  applyQuad();
}

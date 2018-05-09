#include "BloomEffect.hpp"
#include "Engine/Renderer/Geometry/Vertex.hpp"
#include "Engine/Renderer/Renderer.hpp"

BloomEffect::BloomEffect(Renderer& renderer, RenderTarget& target, 
                         const Shader& bloomShader, Texture& bloomTex)
  : AfterEffect(renderer, target, bloomShader)
  , mBloomTex(bloomTex) {
  EXPECTS(target.dimension() == bloomTex.dimension());
}

void BloomEffect::preEffect() {

   RenderTarget* current = &mBloomTex.asRenderTarget();
   RenderTarget* scratch = &mEffectTarget;
   for(uint i = 0; i < 6; i++) {
     mRenderer.setShader(mEffectShader, 0); // blur shader
     mRenderer.setTexture(TEXTURE_DIFFUSE, current);
     mEffectCamera.setColorTarget(scratch);
     mRenderer.setCamera(&mEffectCamera);
     applyQuad();

     mRenderer.setShader(mEffectShader, 1); // blur shader
     mRenderer.setTexture(TEXTURE_DIFFUSE, scratch);
     mEffectCamera.setColorTarget(current);
     mRenderer.setCamera(&mEffectCamera);
     applyQuad();
   }
  
   if(scratch != &mEffectTarget) {
     mRenderer.copyTexture(scratch, &mEffectTarget);
   }

}

void BloomEffect::effect() {
  
  mEffectCamera.setColorTarget(&mTarget);
  mRenderer.setTexture(TEXTURE_DIFFUSE, &mEffectTarget);

  mRenderer.setShader(mEffectShader, 2); // mix shader
  mRenderer.setCamera(&mEffectCamera);

  applyQuad();
}

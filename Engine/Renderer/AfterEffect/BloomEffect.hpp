#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/AfterEffect/AfterEffect.hpp"

class Texture;

class BloomEffect: public AfterEffect {
public:

  BloomEffect(Renderer& renderer, RenderTarget& target, const Shader& bloomShader, Texture& bloomTex);

protected:

  // blur the bloomTex for several times
  virtual void preEffect() override;
  virtual void effect() override;

  // additive result of blur(which is stored in the effectTarget)
  // void effect() override;

  Texture& mBloomTex;
};

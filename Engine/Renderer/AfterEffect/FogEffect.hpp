#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/AfterEffect/AfterEffect.hpp"

class FogEffect: public AfterEffect {
public:
public:
  FogEffect(Renderer& renderer, RenderTarget& target, const Shader& effectShader, const Texture& depthTex);

protected:
  void effect() override;

  const Texture& mDepthTex;
  
};

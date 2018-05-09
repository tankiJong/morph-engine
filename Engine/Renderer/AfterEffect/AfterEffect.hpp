#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderTarget.hpp"

class Shader;

// all effect class should perform as a non side effect step. which means it only effect the target render target
class AfterEffect {
public:
  virtual ~AfterEffect() = default;

  AfterEffect(Renderer& renderer, RenderTarget& target, const Shader& effectShader);;
  void apply();
protected:
  Renderer& mRenderer;
  RenderTarget& mTarget;
  const Shader* mEffectShader = nullptr;
  RenderTarget mEffectTarget;

  Camera mEffectCamera;
  virtual void effect() = 0;
  virtual void preEffect() {};
  virtual void postEffect() {};

  void applyQuad();
};
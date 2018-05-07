#pragma once
#include "Engine/Core/common.hpp"

class RenderScene;
class Camera;
class Renderer;
class Texture;
class RenderTask;
class Light;
class RenderTarget;
class ForwardRendering {
public:
  ForwardRendering(Renderer* renderer);;
  ~ForwardRendering();
  void render(RenderScene& scene);
protected:
  void renderView(RenderScene& scene, Camera& cam);
  void createShadowMap(Light& light, span<RenderTask> tasks);
  void prepass(RenderScene& scene, span<RenderTask> tasks, Camera& cam);
  void pass(RenderScene& scene, span<RenderTask> tasks, Camera& cam);
  void postpass(RenderScene& scene, span<RenderTask> tasks, Camera& cam);
  Renderer* mRenderer;

  owner<RenderTarget*> mBloomTexture = nullptr;
  owner<RenderTarget*> mShadowInfo = nullptr;

};

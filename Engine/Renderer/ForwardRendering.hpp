#pragma once
#include "Engine/Core/common.hpp"

class RenderScene;
class Camera;
class Renderer;
class Texture;
class RenderTask;

class ForwardRendering {
public:
  ForwardRendering(Renderer* renderer): mRenderer(renderer) {};
  void render(RenderScene& scene);
protected:
  void renderView(RenderScene& scene, Camera& cam);
  void prepass(RenderScene& scene, span<RenderTask> tasks, Camera& cam);
  void pass(RenderScene& scene, span<RenderTask> tasks, Camera& cam);
  void postpass(RenderScene& scene, span<RenderTask> tasks, Camera& cam);
  Renderer* mRenderer;

  Texture* mBloomTexture;
};

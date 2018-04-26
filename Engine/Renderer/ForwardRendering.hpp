#pragma once
#include "Engine/Core/common.hpp"

class RenderScene;
class Camera;
class Renderer;

class ForwardRendering {
public:
  ForwardRendering(Renderer* renderer): mRenderer(renderer) {};
  void render(RenderScene& scene);
protected:
  void renderView(RenderScene& scene, Camera* cam);
  Renderer* mRenderer;
};

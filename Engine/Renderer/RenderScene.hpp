#pragma once
#include "Engine/Core/common.hpp"
#include <vector>

class Renderable;
class Light;
class Camera;
class vec3;
class RenderScene {
public:
  void add(Renderable& r);
  void add(Light& l);

  // add and sort the camera immediately
  void set(Camera& c);

  void remove(Renderable& r);
  void remove(Light& l);

  Camera* camera() const { return mCamera; }
  span<Light* const> lights() const { return mLights; }
  span<Renderable* const> Renderables() const { return mRenderables; }

protected:
  std::vector<Renderable*> mRenderables;
  std::vector<Light*> mLights;
  Camera* mCamera;
};

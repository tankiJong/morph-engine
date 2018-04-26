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
  void add(Camera& c);

  void remove(Renderable& r);
  void remove(Light& l);
  void remove(Camera& c);

  span<Camera* const> cameras() const { return mCameras; }
  span<Light* const> lights() const { return mLights; }
  span<Renderable* const> Renderables() const { return mRenderables; }

  void lightContributorsAt(const vec3& position, uint* out_lights, uint* out_count) const;

  void sortCamera();

protected:
  std::vector<Renderable*> mRenderables;
  std::vector<Light*> mLights;
  std::vector<Camera*> mCameras;
};

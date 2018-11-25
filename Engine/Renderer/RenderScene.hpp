#pragma once
#include "Engine/Core/common.hpp"
#include <vector>
#include "Engine/Graphics/RHI/RHIBuffer.hpp"

class Renderable;
class Light;
class Camera;
class vec3;
class BVH;

class RenderScene {
public:
  void add(Renderable& r);
  void add(Light& l);

  // add and sort the camera immediately
  void set(Camera& c);
  void set(BVH* bvh);
  void remove(Renderable& r);
  void remove(Light& l);

  Camera* camera() const { return mCamera; }
  span<Light* const> lights() const { return mLights; }
  span<Renderable* const> Renderables() const { return mRenderables; }
  BVH* bvh() const { return mBvh; }


protected:
  std::vector<Renderable*> mRenderables;
  std::vector<Light*> mLights;
  Camera* mCamera;
  BVH* mBvh;
};

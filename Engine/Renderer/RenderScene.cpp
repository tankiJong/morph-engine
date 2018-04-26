#include "RenderScene.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Math/Primitives/vec4.hpp"
#include "Engine/Framework/Light.hpp"
#include "Engine/Math/Curves.hpp"
#include "Engine/Renderer/Camera.hpp"

template<typename T>
inline bool notInList(std::vector<T*> list, T* ele) {
  return std::find(list.begin(), list.end(), ele) == list.end();
}

template<typename T>
inline T* removeAndShrink(std::vector<T*> list, T* ele) {
  auto loc = std::find(list.begin(), list.end(), ele);
  if(loc == list.end()) return nullptr;

  auto ret = *loc;
  *loc = list.back();
  list.pop_back();

  return ret;
}

void RenderScene::add(Renderable& r) {
  EXPECTS(notInList(mRenderables, &r));

  mRenderables.push_back(&r);
}

void RenderScene::add(Light& l) {
  EXPECTS(notInList(mLights, &l));

  mLights.push_back(&l);
}

void RenderScene::add(Camera& c) {
  EXPECTS(notInList(mCameras, &c));

  mCameras.push_back(&c);
}

void RenderScene::remove(Renderable& r) {
  removeAndShrink(mRenderables, &r);
}

void RenderScene::remove(Light& l) {
  removeAndShrink(mLights, &l);
}

void RenderScene::remove(Camera& c) {
  removeAndShrink(mCameras, &c);
}

void RenderScene::lightContributorsAt(const vec3& position, uint* out_lights, uint* out_count) const {
  if (out_lights == nullptr && out_count == nullptr) return;

  struct indexedLight {
    Light* l = nullptr;
    size_t index = 0;
  };
  std::vector<indexedLight> lights;

  lights.reserve(mLights.size());

  for(size_t i = 0; i < mLights.size(); i++) {
    lights.push_back({mLights[i], i});
  }

  std::sort(lights.begin(), lights.end(), [&position](const indexedLight& a, const indexedLight& b) {
    return position.distance2(a.l->transform.position()) < position.distance2(b.l->transform.position());
  });

  uint lightCount = clamp<uint>(lights.size(), 0u, NUM_MAX_LIGHTS);

  if(out_count) {
    *out_count = lightCount;
  }

  if(out_lights) {
    for(uint i = 0; i < lightCount; i++) {
      out_lights[i] = lights[i].index;
    }
  }
}

void RenderScene::sortCamera() {
  static auto comp = [](Camera* a, Camera* b) {
    return a->sort < b->sort;
  };
  if (std::is_sorted(mCameras.begin(), mCameras.end(), comp)) return;
  std::sort(mCameras.begin(), mCameras.end(), comp);
}


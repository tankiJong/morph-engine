#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/type.h"
#include "Engine/Renderer/RenderTarget.hpp"
#include "Engine/Graphics/Camera.hpp"

class Rgba;
enum eLightType {
  LIGHT_UNKNOWN,
  LIGHT_SPOT,
  LIGHT_DIRECTIONAL,
  LIGHT_POINT,
  NUM_LIGHT_TYPE,
};
class Light {
public:
  Transform transform;
  bool castShadow = false;
  eLightType type = LIGHT_UNKNOWN;

  void asDirectionalLight(float intensity = 1.f, const vec3& attenuation = vec3(0, 0, 1), 
                          const Rgba& color = Rgba::white);

  void asPointLight(float intensity = 1.f, const vec3& attenuation = vec3(0, 0, 1), 
                    const Rgba& color = Rgba::white);

  void asSpotLight(float innerAngle, float outerAngle,
                   float intensity = 1.f, const vec3& attenuation = vec3(0, 0, 1),
                   const Rgba& color = Rgba::white);

  light_info_t& info();

  // RenderTarget& shadowMap();
  // Camera& camera();
  float fovAngle() const;

  float attenuation(vec3 position) const;
  // void updateCamera(const Camera& view);
  static constexpr uint SHADOW_MAP_SIZE = 1024u;
protected:
  void updateCamera();
  light_info_t mInfo;
  // RenderTarget* mShadowMap = nullptr;
  Camera mCamera;
};

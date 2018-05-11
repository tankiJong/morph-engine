#include "Engine/Framework/Light.hpp"
#include "Engine/Math/MathUtils.hpp"
void Light::asDirectionalLight(float intensity, const vec3& attenuation, const Rgba& color) {
  mInfo.asDirectionalLight(transform.position(), transform.forward(), intensity, attenuation, color);
  type = LIGHT_DIRECTIONAL;
  updateCamera();
}

void Light::asPointLight(float intensity, const vec3& attenuation, const Rgba& color) {
  mInfo.asPointLight(transform.position(), intensity, attenuation, color);
  type = LIGHT_POINT;
  updateCamera();
}

void Light::asSpotLight(float innerAngle, float outerAngle, float intensity, const vec3& attenuation, const Rgba& color) {
  mInfo.asSpotLight(transform.position(), transform.forward(), innerAngle, outerAngle, intensity, attenuation, color);
  type = LIGHT_SPOT;
  updateCamera();
}

light_info_t& Light::info() {
  mInfo.vp = mCamera.projection() * mCamera.view();
  mInfo.position = transform.position();
  mInfo.direction = transform.forward();
  return mInfo;
}

RenderTarget& Light::shadowMap() {
  if(mShadowMap == nullptr) {
    mShadowMap = new RenderTarget(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, TEXTURE_FORMAT_D24S8);
  }
  return *mShadowMap;
}

Camera& Light::camera() {
  mCamera.setDepthStencilTarget(&shadowMap());
  mCamera.transfrom() = transform;

  return mCamera;
}

float Light::fovAngle() const {
  return acosDegrees(mInfo.dotOuterAngle);
}

float Light::attenuation(vec3 position) const {
  float distance = transform.position().distance(position);

  return mInfo.color.a / (mInfo.attenuation.x + mInfo.attenuation.y * distance + mInfo.attenuation.z * distance * distance);
}

void Light::updateCamera() {
  float fz = 100.f, nz = 0.01f;
  vec2 size(100.f, 100.f);

  switch (type) {
    case LIGHT_UNKNOWN: return;
    case LIGHT_SPOT:
      mCamera.setProjectionPrespective(fovAngle()*2, 5.f, 5.f, nz, fz);
      break;
    case LIGHT_DIRECTIONAL:
      mCamera.setProjectionOrtho(size.x, size.y, nz, fz);
      break;
    case LIGHT_POINT: return;
    case NUM_LIGHT_TYPE: return;
    default:;
  }
}


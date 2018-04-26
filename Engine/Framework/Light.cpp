#include "Engine/Framework/Light.hpp"

void Light::asDirectionalLight(float intensity, const vec3& attenuation, const Rgba& color) {
  info.asDirectionalLight(transform.position(), transform.forward(), intensity, attenuation, color);
}

void Light::asPointLight(float intensity, const vec3& attenuation, const Rgba& color) {
  info.asPointLight(transform.position(), intensity, attenuation, color);
}

void Light::asSpotLight(float innerAngle, float outerAngle, float intensity, const vec3& attenuation, const Rgba& color) {
  info.asSpotLight(transform.position(), transform.forward(), innerAngle, outerAngle, intensity, attenuation, color);
}

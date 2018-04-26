#include "type.h"
#include "Engine/Math/Curves.hpp"

void light_info_t::asDirectionalLight(const vec3& pos,
                                             const vec3& dir,
                                             float intensity,
                                             const vec3& atten,
                                             const Rgba& col) {
  asSpotLight(pos, dir, 90.f, 90.f, intensity, atten, col);
}

void light_info_t::asPointLight(const vec3& pos, float intensity, const vec3& atten, const Rgba& col) {
  this->position = pos;
  this->direction = vec3::zero;
  this->directionFactor = 0.f;
  
  this->color = col.normalized();
  this->color.a = intensity;
  
  this->attenuation = atten;
  this->specAttenuation = atten;
  
  this->dotInnerAngle = 1.f; // cos0
  this->dotOuterAngle = 0.f; // cos90
}

void light_info_t::asSpotLight(const vec3& pos,
                                      const vec3& dir,
                                      float innerAngle,
                                      float outerAngle,
                                      float intensity,
                                      const vec3& atten,
                                      const Rgba& col) {
  this->position = pos;
  this->direction = dir;
  this->directionFactor = 1.f;

  this->color = col.normalized();
  this->color.a = intensity;
  
  this->attenuation = atten;
  this->specAttenuation = atten;
  
  this->dotInnerAngle = cosDegrees(innerAngle);
  this->dotOuterAngle = cosDegrees(outerAngle);
}
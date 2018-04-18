#ifndef DEFINE_LIGHT_F
#define DEFINE_LIGHT_F

#include "./common.glsl"
#include "./math.glsl"
#define NUM_MAX_LIGHT 8

struct light_t {
  vec4 color;

  vec3 attenuation;
  float dotInnerAngle;

  vec3 specAttenuation;
  float dotOuterAngle;

  vec3 position;
  float directionFactor;

  vec3 direction;
  float _pad00;
};

struct light_buffer_t {
  vec4 ambience;
  light_t lights[NUM_MAX_LIGHT];
};

layout(std140, binding = UNIFORM_LIGHT) uniform Light {
  light_buffer_t lights;
};

/*
              Point             Directional      Spot

dir         lpos - wpos            -ldir          lpos - wpos     - need normalize
*/

vec3 IncidenceDirection(light_t from, vec3 toPosition) {
  vec3 dir = from.direction;
  vec3 pointLightDir =  toPosition - from.position;

  return normalize(mix(pointLightDir, dir, from.directionFactor));
}

vec3 Ambient(vec4 ambience) {
  return ambience.xyz * ambience.w;
}

float Attenuation(float intensity, float dis, vec3 factor) {
  float atten = intensity / ( factor.x 
      + factor.y * dis 
      + factor.z * dis * dis);

  return clamp(atten, 0.f, 1.f);
}

float LightPower(light_t light, vec3 surfacePosition) {
  vec3 inDirection = IncidenceDirection(light, surfacePosition);

  float dotAngle = dot(inDirection, normalize(surfacePosition - light.position));
  return smoothstep(light.dotOuterAngle, light.dotInnerAngle, dotAngle);
}

vec3 Diffuse(light_t light, vec3 surfacePosition, vec3 surfaceNormal) {
  vec3 inDirection = IncidenceDirection(light, surfacePosition);
  float dot3 = dot(-inDirection, surfaceNormal);

  float lightPower = LightPower(light, surfacePosition);
  
  float dis = distance(light.position, surfacePosition);
  float atten = Attenuation(light.color.w, dis, light.attenuation);
  return lightPower * dot3 * atten * light.color.xyz;
}

vec3 Diffuse(light_buffer_t lights,
             vec3 surfacePosition, vec3 surfaceNormal) {
  // dot3 diffuse
  vec3 diffuse = vec3(0);

  for(int i = 0; i < NUM_MAX_LIGHT; i++) {
    diffuse += Diffuse(lights.lights[i], surfacePosition, surfaceNormal);
  }

  return clamp(diffuse, vec3(0), vec3(1));
}

vec3 Specular(light_t light, vec3 surfacePosition, vec3 surfaceNormal, float shininesss, float smoothness, vec3 eyeDir) {
  vec3 inDirection = IncidenceDirection(light, surfacePosition);
  vec3 r = reflect(inDirection, surfaceNormal);

  
  float factor = max(0.f, dot(eyeDir, r));
  factor = shininesss * pow(factor, smoothness);

  factor *= LightPower(light, surfacePosition);

  float dis = distance(light.position, surfacePosition);
  float atten = Attenuation(light.color.w, dis, light.attenuation);

  return factor * atten * light.color.xyz;
}

vec3 Specular(light_buffer_t lights, vec3 surfacePosition, vec3 surfaceNormal, float shininesss, float smoothness, vec3 eyeDir) {
  vec3 specular = vec3(0);

  for(int i = 0; i < NUM_MAX_LIGHT; i++) {
    specular += Specular(lights.lights[i], surfacePosition, surfaceNormal, shininesss, smoothness, eyeDir);
  }

  return specular;
}

vec4 PhongLighting(light_buffer_t lights, 
                   vec3 surfacePosition, vec3 surfaceNormal, float shininesss, float smoothness, vec4 surfaceColor,
                   vec3 eyeDir) {

  #ifndef _disable_diffuse
  vec3 diffuse = Diffuse(lights, surfacePosition, surfaceNormal);
  #else
  vec3 diffuse = vec3(0);
  #endif
  
  #ifndef _disable_specular
  vec3 specular = Specular(lights, surfacePosition, surfaceNormal, shininesss, smoothness, eyeDir);
  #else
  vec3 specular = vec3(0);
  #endif

  #ifndef _disable_ambient
  vec3 surfaceLight = Ambient(lights.ambience) + diffuse;
  #else
  vec3 surfaceLight = diffuse;
  #endif

  clamp(surfaceLight, vec3(0), vec3(1));

  #ifdef _d_light_only
  vec4 finalColor = vec4(surfaceLight, 1) * vec4(1) + vec4(specular, 0);
  #else
  vec4 finalColor = vec4(surfaceLight, 1) * surfaceColor + vec4(specular, 0);
  #endif

  return clamp(finalColor, vec4(0), vec4(1));
}


#endif
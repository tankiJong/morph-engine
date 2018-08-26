#ifndef __LIGHT_INCLUDED__
#define __LIGHT_INCLUDED__
#include "Resource.hlsl"

float3 InDirection(light_info_t light, float3 surfacePosition) 
{
	float3 direction = surfacePosition - light.position;
	
  return normalize(lerp(direction, light.direction, light.directionFactor));
}

float LightPower(light_info_t light, float3 surfacePosition)
{
	float3 inDirection = InDirection(light, surfacePosition);
	float dotAngle = dot(inDirection, normalize(surfacePosition - light.position));

	return smoothstep(light.dotOuterAngle, light.dotInnerAngle, dotAngle);
}

float Attenuation(float intensity, float dis, float3 factor) 
{
	float atten = intensity / (factor.x*factor.x*dis + factor.y*dis + dis);
	return clamp(atten, 0.f, 1.f);
}

float3 Diffuse(float3 surfacePosition, float3 surfaceNormal, float3 surfaceColor, light_info_t light) 
{
  float3 inDirection = InDirection(light, surfacePosition);

  float dot3 = max(0, dot(-inDirection, surfaceNormal));

	float lightPower = LightPower(light, surfacePosition);

	float dis = distance(light.position, surfacePosition);
	float atten = Attenuation(light.color.w, dis, light.attenuation);

	float3 color = dot3 * lightPower * atten * surfaceColor;
  return color;
}

float Specular(float3 surfacePosition, float3 surfaceNormal, float3 eyeDireciton, 
							 float specularAmount, float specularPower, light_info_t light)	{
	float3 inDirection = InDirection(light, surfacePosition);								 
	float3 r = reflect(inDirection, surfaceNormal);

	float factor = max(0.f, dot(eyeDireciton, r));
	factor = specularAmount * pow(factor, specularPower);
	factor *= LightPower(light, surfacePosition);

	float dis = distance(light.position, surfacePosition);
	float atten = Attenuation(light.color.a, dis, light.attenuation);

	return factor * atten * light.color.rgb;
}

float3 PhongLighting(float3 surfacePosition, float3 surfaceNormal, float3 surfaceColor, float3 eyePosition)
{
	const float SPECULAR_AMOUNT = 5.f;
	const float SPECULAR_POWER = 3.f;
  float3 ambient = .5f;
  float3 diffuse = Diffuse(surfacePosition, surfaceNormal, surfaceColor, light);
  float3 specular = Specular(surfacePosition, surfaceNormal, 
														 normalize(eyePosition - surfacePosition), SPECULAR_AMOUNT, SPECULAR_POWER, light);

  float3 color = ambient * surfaceColor + diffuse + specular;

  return clamp(color, float3(0.f, 0.f, 0.f), float3(1.f, 1.f, 1.f));
}

#endif
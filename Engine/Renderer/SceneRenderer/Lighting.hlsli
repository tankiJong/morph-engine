#include "Common.hlsli"


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

float3 Specular(float3 surfacePosition, float3 surfaceNormal, float3 eyeDireciton, 
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
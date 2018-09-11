#include "Common.hlsli"

#define DeferredLighting_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 6), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"



Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
Texture2D<float4> gTexAO: register(t15);

RWTexture2D<float4> uTexScene: register(u0);

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

float3 Ambient(uint2 pix) {
	return gTexAO[pix].xyz;
}

float3 PhongLighting(uint2 pix)
{
	float3 surfacePosition = gTexPosition[pix].xyz;
	float3 surfaceNormal = gTexNormal[pix].xyz;
	float3 surfaceColor = gTexAlbedo[pix].xyz;
	float3 eyePosition;
	{
		float4 eye = mul(inverse(view), mul(inverse(projection), float4(0,0,0,1.f)));
		eyePosition = eye.xyz / eye.w;
	}
  float3 ambient = Ambient(pix);

	const float SPECULAR_AMOUNT = 5.f;
	const float SPECULAR_POWER = 3.f;
  float3 diffuse = Diffuse(surfacePosition, surfaceNormal, surfaceColor, gLight);
  float3 specular = Specular(surfacePosition, surfaceNormal, 
														 normalize(eyePosition - surfacePosition), SPECULAR_AMOUNT, SPECULAR_POWER, gLight);

  float3 color = ambient * surfaceColor + diffuse + specular;

  return clamp(color, float3(0.f, 0.f, 0.f), float3(1.f, 1.f, 1.f));
}

[RootSignature(DeferredLighting_RootSig)]
[numthreads(32, 32, 1)]
void main( uint3 threadId: SV_DispatchThreadID )
{
	uint2 pix = threadId.xy;
	uTexScene[pix] = float4(PhongLighting(pix), 1.f);
}
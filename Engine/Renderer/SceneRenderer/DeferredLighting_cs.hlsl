#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"

#define DeferredLighting_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 6), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 3), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"



Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
Texture2D<float4> gTexAO: register(t15);


RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<uint> uNumSurfels: register(u1);
RWTexture2D<float4> uTexScene: register(u2);


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

	const float SPECULAR_AMOUNT = 1.f;
	const float SPECULAR_POWER = 2.f;
  float3 diffuse = Diffuse(surfacePosition, surfaceNormal, surfaceColor, gLight);
  float3 specular = Specular(surfacePosition, surfaceNormal, 
														 normalize(eyePosition - surfacePosition), SPECULAR_AMOUNT, SPECULAR_POWER, gLight);

	float3 indirect = float3(0,0,0);
	uint count = uNumSurfels[0];
	float finalFactor = 0;
	for(uint i = 0; i < count; i++) {
		float factor = clamp(dot(uSurfels[i].normal, surfaceNormal), 0, 1);
		factor = factor * factor;
		float d = distance(surfacePosition, uSurfels[i].position);
		factor = factor / (1 + factor * d*d);
		indirect +=	uSurfels[i].indirectLighting * factor;
		finalFactor += factor;
	}
	
  float3 color = diffuse + indirect * ambient/* + specular*/;

  return clamp(color, float3(0.f, 0.f, 0.f), float3(1.f, 1.f, 1.f));
}

[RootSignature(DeferredLighting_RootSig)]
[numthreads(32, 32, 1)]
void main( uint3 threadId: SV_DispatchThreadID )
{
	uint2 pix = threadId.xy;

	float3 direct = 0;
	float3 indirect = 0;
	uint count = uNumSurfels[0];

	uTexScene[pix] = float4(PhongLighting(pix), 1.f);
}
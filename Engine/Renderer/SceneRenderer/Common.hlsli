#ifndef __SR_COMMON_INCLUDED__
#define __SR_COMMON_INCLUDED__

#include "../Shader/Math.hlsl"
#include "../Shader/Random.hlsli"
#include "Bvh.hlsli"

#define RootSig_Common "DescriptorTable(CBV(b0, numDescriptors = 4), SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL),"

#define M_PI 3.141592653

struct vertex_t {
	float4 position;
	float4 color;
};



struct light_info_t {
  float4 color;

  float3 attenuation;
  float dotInnerAngle;

  float3 specAttenuation;
  float dotOuterAngle;

  float3 position;
  float directionFactor;

  float3 direction;
  float __pad00;

  float4x4 vp;
};

struct DisneyBRDFParam {
	float subsurface;
	float specular;
	float specularTint;
	float anisotropic;
	float sheen;
	float sheenTint;
	float clearcoat;
	float clearcoatGloss;
};

cbuffer cFrameData: register(b0) {
	float gTime;
	float gFrameCount;
	float gRoughness;
	float gMetallic;
}

cbuffer cCamera : register(b1) {
  float4x4 projection;
  float4x4 view;
	float4x4 prev_projection;
  float4x4 prev_view;
};

cbuffer cModel: register(b2) {
	float4x4 model;
}

cbuffer cLight: register(b3) {
	light_info_t gLight;
}


Texture2D gTexAmbient: register(t0);
SamplerState gSampler : register(s0);

	

bool outScreen(uint2 pix, uint2 size) {
	return pix.x >= size.x || pix.y >= size.y;
}

// http://www.chilliant.com/rgb2hsv.html

float3 HUEtoRGB(in float H)
{
  float R = abs(H * 6 - 3) - 1;
  float G = 2 - abs(H * 6 - 2);
  float B = 2 - abs(H * 6 - 4);
  return saturate(float3(R,G,B));
}

static const float Epsilon = 1e-10;
 
float3 RGBtoHCV(in float3 RGB)
{
  // Based on work by Sam Hocevar and Emil Persson
  float4 P = (RGB.g < RGB.b) ? float4(RGB.bg, -1.0, 2.0/3.0) : float4(RGB.gb, 0.0, -1.0/3.0);
  float4 Q = (RGB.r < P.x) ? float4(P.xyw, RGB.r) : float4(RGB.r, P.yzx);
  float C = Q.x - min(Q.w, Q.y);
  float H = abs((Q.w - Q.y) / (6 * C + Epsilon) + Q.z);
  return float3(H, C, Q.x);
}

float3 HSLtoRGB(in float3 HSL)
{
  float3 RGB = HUEtoRGB(HSL.x);
  float C = (1 - abs(2 * HSL.z - 1)) * HSL.y;
  return (RGB - 0.5) * C + HSL.z;
}


float3 RGBtoHSL(in float3 RGB)
{
  float3 HCV = RGBtoHCV(RGB);
  float L = HCV.z - HCV.y * 0.5;
  float S = HCV.y / (1 - abs(L * 2 - 1) + Epsilon);
  return float3(HCV.x, S, L);
}

float ExtractLfromRGB(in float3 RGB) {
	return dot(float3(0.2126, 0.7152, 0.0722), RGB);
}


#define SCREEN_SIZE_FROM(size, tex) uint2 size; tex.GetDimensions(size.x, size.y)

#define RETURN_IF_OUT_TEX(pix, tex) {SCREEN_SIZE_FROM(size, tex); if(outScreen(pix, size))	return;}



Ray GenPrimaryRay(float3 ndc) {
	Ray ray;
	
	float4x4 invView = inverse(view);
	float4 _worldCoords = mul(invView, mul(inverse(projection), float4(ndc, 1.f)));
	float3 worldCoords = _worldCoords.xyz / _worldCoords.w;

	ray.position = worldCoords;

	float3 origin = mul(invView, float4(0, 0, 0, 1.f)).xyz;
	ray.direction = normalize(worldCoords - origin);
	ray.directionInv = 1.f / ray.direction;

	return ray;
}


float3 GetRandomDirection(inout uint seed, float3 normal) {
	
	Randomf r = rnd01(seed);
	seed = r.seed; 
	float x = (r.value - .5f)*2.f;
						
	float3 right = float3(x,1,1);
	float3 _tan = normalize(right);
	float3 bitan = normalize(cross(_tan, normal));
	float3 tan = cross(bitan, normal);

	float3x3 tbn = transpose(float3x3(tan, normal, bitan));

	r = rnd01(seed); 
	seed = r.seed;
	float b = r.value; // cosTheta -> y

	r = rnd01(seed);
	seed = r.seed;
	float sinTheta = sqrt(1 - b*b);
	float phi = 2 * 3.1415926f * r.value;

	float a = sinTheta * cos(phi);
	float c = sinTheta * sin(phi);

	float3 ssample = normalize(mul(tbn, float3(a, b, c)));

	return ssample;
}

Ray GenReflectionRay(inout uint seed, float4 position, float3 normal) {
	
	float3 ssample = GetRandomDirection(seed, normal);

	Ray ray;

	ray.direction = ssample;
	ray.position = position.xyz + 0.0001f * ssample;
	ray.directionInv = 1.f / ray.direction;
	return ray;
}










#endif
#ifndef __SR_COMMON_INCLUDED__
#define __SR_COMMON_INCLUDED__

#include "../Shader/Math.hlsl"
#include "../Shader/Random.hlsli"

#define RootSig_Common "DescriptorTable(CBV(b0, numDescriptors = 4), SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL),"

#define M_PI 3.141592653

struct vertex_t {
	float4 position;
	float4 color;
};


struct Ray {
	float3 position;
	float3 direction;
};

struct Contact {
	float4 position;
	float3 normal;
	float t;
	bool valid;
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

cbuffer cFrameData: register(b0) {
	float gTime;
	float gFrameCount;
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

float HueToRGB(float v1, float v2, float vH) {
  if (vH < 0)
    vH += 1.f;

  if (vH > 1.f)
    vH -= 1.f;

  if ((6.f * vH) < 1.f)
    return (v1 + (v2 - v1) * 6.f * vH);

  if ((2 * vH) < 1)
    return v2;

  if ((3 * vH) < 2)
    return (v1 + (v2 - v1) * ((2.0f / 3.f) - vH) * 6.f);

  return v1;
}

float3 Hsl(float h, float s, float l) {
  float r = 0;
  float g = 0;
  float b = 0;

  if (s == 0) {
    r = g = b = l;
  } else {
    float v1, v2;
    float hue = h / 360.f;

    v2 = (l < 0.5f) ? (l * (1.f + s)) : ((l + s) - (l * s));
    v1 = 2.f * l - v2;

    r = HueToRGB(v1, v2, hue + (1.0f / 3.f));
    g = HueToRGB(v1, v2, hue);
    b = HueToRGB(v1, v2, hue - (1.0f / 3.f));
  }

  return float3(r, g, b);
}



float3 Hue(float hh) {
  return Hsl(hh, 1.f, .5f);
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

	return ray;
}

Contact triIntersection(float3 a, float3 b, float3 c, float color, Ray ray) {

	Contact contact;

	float3 ab = b - a;
	float3 ac = c - a;
	float3 normal = normalize(cross(ac, ab));
	contact.normal = normal;

	contact.valid = dot(normal, ray.direction) < 0;

	float t = (dot(a, normal) - dot(ray.position, normal)) / dot(ray.direction, normal);
	contact.t = t;
	contact.position.xyz = ray.position + t * ray.direction;
	contact.position.w = color;

	float3 p = contact.position.xyz;
	contact.valid = contact.valid && dot(cross(p - a, b - a), normal) >= 0;
	contact.valid = contact.valid && dot(cross(p - b, c - b), normal) >= 0;
	contact.valid = contact.valid && dot(cross(p - c, a - c), normal) >= 0;

	return contact;
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

	float3 sample = normalize(mul(tbn, float3(a, b, c)));

	return sample;
}

Ray GenReflectionRay(inout uint seed, float4 position, float3 normal) {
	
	float3 sample = GetRandomDirection(seed, normal);

	Ray ray;

	ray.direction = sample;
	ray.position = position.xyz + 0.0001f * sample;

	return ray;
}










#endif
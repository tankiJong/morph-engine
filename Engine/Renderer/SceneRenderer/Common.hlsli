#include "../Shader/Math.hlsl"
#include "../Shader/Random.hlsli"

#define RootSig_Common "DescriptorTable(CBV(b0, numDescriptors = 4), SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL),"

struct vertex_t {
	float4 position;
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

cbuffer cTime: register(b0) {
	float gTime;
	float gFrameCount;
}

cbuffer cCamera : register(b1) {
  float4x4 projection;
  float4x4 view;
};

cbuffer cModel: register(b2) {
	float4x4 model;
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
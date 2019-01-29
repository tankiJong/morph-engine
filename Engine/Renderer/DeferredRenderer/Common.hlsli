#ifndef __SR_COMMON_INCLUDED__
#define __SR_COMMON_INCLUDED__

#include "../Shader/Math.hlsl"
#include "../Shader/Random.hlsli"

#define RootSig_Common "DescriptorTable(CBV(b0, numDescriptors = 3), visibility = SHADER_VISIBILITY_ALL),"

#define M_PI 3.141592653

struct vertex_t {
	float4 position;
	float4 color;
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


SamplerState gSampler : register(s0);

	




#endif
#include "../Shader/Math.hlsl"

#define RootSig_Common "DescriptorTable(CBV(b0, numDescriptors = 4), SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL),"

cbuffer cTime: register(b0) {
	float gTime;
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


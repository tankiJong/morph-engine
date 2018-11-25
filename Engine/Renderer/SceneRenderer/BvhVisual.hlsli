#include "Bvh.hlsli"

struct GSInput {
	BVHNode node: BVH_META;
	uint index: VERTEX_INDEX;
};

struct PSInput {
	float4 position: SV_POSITION;
};

cbuffer cCamera : register(b1) {
  float4x4 projection;
  float4x4 view;
	float4x4 prev_projection;
  float4x4 prev_view;
};

#define BvhVisual_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
		"DescriptorTable(CBV(b1, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," 

StructuredBuffer<BVHNode> gBvh: register(t0);

#include "Common.hlsl"
#include "../Shader/Resource.hlsl"
#include "../Shader/Light.hlsl"

struct PSInput {
  float4 position : SV_POSITION;
  float4 color : COLOR;
  float2 uv: UV;
  float3 normal: NORMAL;
	float3 tangent: TANGENT;
	float3 worldPosition: PASS_WORLD;
	float3 eyePosition: PASS_EYE;
};

struct PSOutput {
	float4 color: SV_TARGET0;
	float4 normal: SV_TARGET1;
};

#define GenBuffer_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "DescriptorTable(CBV(b0, numDescriptors = 12), SRV(t0, numDescriptors = 12), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," 
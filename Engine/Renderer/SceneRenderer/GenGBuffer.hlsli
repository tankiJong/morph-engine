#include "Common.hlsli"

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
	float4 position: SV_TARGET2;
};


Texture2D gTexAlbedo:   register(t1);
Texture2D gTexNormal:   register(t2);
Texture2D gTexSpecular: register(t3);

#define GenBuffer_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    RootSig_Common  \
		"DescriptorTable(CBV(b6, numDescriptors = 5), SRV(t1, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," 


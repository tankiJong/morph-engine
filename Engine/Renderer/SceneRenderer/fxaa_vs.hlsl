#include "fullscreen.hlsli"

#define FXAA_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
		"DescriptorTable(SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," 


[RootSignature(FXAA_RootSig)]
PostProcessingVSOutput main(uint vertexId: SV_VertexID) {
	PostProcessingVSOutput output;

	output.tex =  float2((2-vertexId << 1) & 2, 2 - vertexId & 2);
	output.position = float4(output.tex * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return output;
}
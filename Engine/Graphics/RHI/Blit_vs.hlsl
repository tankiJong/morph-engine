#define Blit_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
		"DescriptorTable(SRV(t0, numDescriptors = 1, flags = DATA_VOLATILE), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," 



Texture2D<float4> gTex: register(t0);
SamplerState gSampler : register(s0);

struct PSInput {
	float2 uv: TEXCOORD;
	float4 pos: SV_POSITION;
};
[RootSignature(Blit_RootSig)]
PSInput vmain(uint vertexId: SV_VertexID) {
	PSInput input;
	input.uv = float2((2-vertexId << 1) & 2, 2 - vertexId & 2);
	input.pos = float4(input.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);

	return input;
}

[RootSignature(Blit_RootSig)]
float4 pmain( PSInput input ): SV_Target
{	
	return gTex.SampleLevel(gSampler, input.uv, 0);
}

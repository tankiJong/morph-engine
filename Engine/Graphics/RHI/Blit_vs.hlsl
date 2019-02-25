#define Blit_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
		"DescriptorTable(CBV(b0, numDescriptors = 1), SRV(t0, numDescriptors = 1, flags = DATA_VOLATILE), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," 


cbuffer constriants: register(b0) {
 float2 cOffset;
 float2 cScale;
};

Texture2D<float4> gTex: register(t0);
SamplerState gSampler : register(s0);

struct PSInput {
	float2 uv: TEXCOORD;
	float4 pos: SV_POSITION;
};

static const float2 uvs[6] = {
	float2(0, 0),
	float2(0, 1),
	float2(1, 1),

	float2(0, 0),
	float2(1, 1),
	float2(1, 0),
};

[RootSignature(Blit_RootSig)]
PSInput vmain(uint vertexId: SV_VertexID) {
	PSInput input;
	input.uv = uvs[vertexId];
	input.pos = float4(input.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	
	input.uv = input.uv * cScale + cOffset;
	return input;
}

[RootSignature(Blit_RootSig)]
float4 pmain( PSInput input ): SV_Target
{	
	return gTex.SampleLevel(gSampler, input.uv, 0);
}

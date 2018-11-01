
#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 39
#define FXAA_GREEN_AS_LUMA 1


#include "fxaa.hlsli"


#include "fullscreen.hlsli"

struct PSOutput {
	float4 color: SV_TARGET0;
};


Texture2D<float4> gTexScene: register(t0);
SamplerState gSampler : register(s0);

static const float fxaaSubpix = 1.00;
static const float fxaaEdgeThreshold = 0.2;
static const float fxaaEdgeThresholdMin = 0;



#define FXAA_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
		"DescriptorTable(SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," 


[RootSignature(FXAA_RootSig)]
PSOutput main(PostProcessingVSOutput input) {
	PSOutput output;
	
	float2 fxaaFrame;

	gTexScene.GetDimensions(fxaaFrame.x, fxaaFrame.y);
	
	FxaaTex tex = { gSampler, gTexScene };
	
	output.color = FxaaPixelShader(input.tex, 0, tex, tex, tex, 1 / fxaaFrame, 0, 0, 0,
									fxaaSubpix, fxaaEdgeThreshold, fxaaEdgeThresholdMin, 0, 0, 0, 0);

	// output.color = float4(1, 0, 0, 1);
	return output;
}
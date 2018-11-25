#include "BvhVisual.hlsli"

struct PSOutput {
	float4 color: SV_TARGET0;
};

[RootSignature(BvhVisual_RootSig)]
PSOutput main(PSInput input) {

	PSOutput output;
	output.color = float4(1, 1, 1, 0.01);
	return output;
}
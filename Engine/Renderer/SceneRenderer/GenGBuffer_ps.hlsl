#include "GenGBuffer.hlsli"

[RootSignature(GenBuffer_RootSig)]
PSOutput main(PSInput input)
{
	PSOutput output;

	// float4 texColor = input.color * gTexAlbedo.Sample(gSampler, input.uv);
	// float4 texColor = input.color;
  // output.color = float4(
	// 	PhongLighting(input.worldPosition, input.normal, texColor.xyz, input.eyePosition), 1.f);


	output.color = input.color;
	output.normal = float4(input.normal * .5f + .5f, 1.f);
	output.position = float4(input.worldPosition, 1.f);
	output.velocity = float4(input.deltaViewOffset.xyz / gTime, 1.f);
	return output;
}
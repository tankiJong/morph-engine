
#define Blur_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
		"DescriptorTable(CBV(b0, numDescriptors = 1), SRV(t0, numDescriptors = 1), UAV(u0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, filter = FILTER_ANISOTROPIC, maxAnisotropy = 1, visibility = SHADER_VISIBILITY_PIXEL)," 

cbuffer cBlurConstant: register(b0) {
	float cVerticalPass;
	float3 __padding;
};

Texture2D<float4> gInput: register(t0);
RWTexture2D<float4> uOutput: register(u0);

static const int KERNEL_SIZE = 9;

[RootSignature(Blur_RootSig)]
[numthreads(8, 8, 1)]
void main( uint3 pixCoords : SV_DispatchThreadID )
{
	float weight[KERNEL_SIZE] = { 0.016216, 0.054054, 0.1216216, 0.1945946, 
											0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
	uint2 dim;

	gInput.GetDimensions(dim.x, dim.y);

	if(pixCoords.x >= dim.x || pixCoords.y >= dim.y) return;
	
	int2 dir;
	if(cVerticalPass)	{
		dir = int2(0, 1);
	}	else {
		dir = int2(1, 0);
	}
	
	int2 startCoords = int2(pixCoords.xy) - KERNEL_SIZE / 2 * dir;



	float3 color = 0.f.xxx;
	float totalWeight = 0; 
	for(int i = 0; i < KERNEL_SIZE; i++) {
		startCoords = clamp(startCoords, 0.f.xx, dim);
		color += weight[i] * gInput[startCoords].xyz;
		totalWeight += weight[i];
		startCoords += dir;
	}
	// return;

	uOutput[pixCoords.xy] = float4(color / totalWeight, 1.f);
}
#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"

#define DeferredLighting_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 6), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 3), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"



Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<SurfelBucketInfo> uSurfelBucket: register(u1);
RWTexture2D<float4> uTexIndirect: register(u2);

float4 ComputeIndirect(uint2 pix)
{
	float3 surfacePosition = gTexPosition[pix].xyz;
	float3 surfaceNormal = gTexNormal[pix].xyz * 2.f - 1.f;
	float3 surfaceColor = gTexAlbedo[pix].xyz;

	float3 indirect = float3(0,0,0);
	 
	float total = 0;

	uint bucketCount, _;
	uSurfelBucket.GetDimensions(bucketCount, _);

	for(uint k = 0; k <= bucketCount; k++) {
			SurfelBucketInfo info = uSurfelBucket[k];
	 	
			uint i = info.startIndex;
			while(i < info.startIndex + info.currentCount) {
				surfel_t surfel = uSurfels[i];
				float2 d = GetProjectedDistanceFromSurfel(surfacePosition, surfel);

				float weight = exp(-1.f * (d.x * d.x) / (2 * 3 * SURFEL_RADIUS * 3 * SURFEL_RADIUS)) / (d.y * d.y / (SURFEL_RADIUS * SURFEL_RADIUS) + 1.f);
				float iscovered = (dot(surfaceNormal, surfel.normal) > .9f ? 1.f : 0.f) * weight;
				total += weight;
				indirect =	mad( surfel.indirectLighting, iscovered, indirect );

				i++;
		}
	}

	return float4(indirect, total);
}

[RootSignature(DeferredLighting_RootSig)]
[numthreads(32, 32, 1)]
void main( uint3 threadId: SV_DispatchThreadID )
{
	uint2 pix = threadId.xy;

	uint2 size;
	uTexIndirect.GetDimensions(size.x, size.y);
	if(pix.x > size.x || pix.y > size.y) return;


	float4 color = ComputeIndirect(pix * 2);
	uTexIndirect[pix] = color;

}


#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"

#define DeferredLighting_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 6), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 3, flags=DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"


Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<SurfelBucketInfo> uSurfelBucket: register(u1);
RWTexture2D<float4> uTexIndirect: register(u2);

static uint groupOffset;
static uint3 localThreadId;
#define groupSliceSize 64

groupshared SurfelBucketInfo infos[64]; // = groupSliceSize
groupshared surfel_t surfeCache[8];

float4 ComputeIndirect(uint2 pix)
{
	float3 surfacePosition = gTexPosition[pix].xyz;
	float3 surfaceNormal = gTexNormal[pix].xyz * 2.f - 1.f;
	float3 surfaceColor = gTexAlbedo[pix].xyz;

	float3 indirect = float3(0,0,0);
	 
	float total = 0;

	uint bucketCount, _;
	uSurfelBucket.GetDimensions(bucketCount, _);

	if (groupOffset * groupSliceSize + localThreadId.x < bucketCount) {
		infos[localThreadId.x] = uSurfelBucket[groupOffset * groupSliceSize + localThreadId.x];
	}	
	GroupMemoryBarrierWithGroupSync();

	// [unroll]
	for(uint k = 0; k < groupSliceSize; k++) {
		
		// SurfelBucketInfo info = infos[k];

		uint i = 0;

		while(i < infos[k].currentCount) {
			surfeCache[localThreadId.y] = uSurfels[infos[k].startIndex + i + localThreadId.y];
			GroupMemoryBarrierWithGroupSync();

			uint count = min(4, infos[k].currentCount - i);
			uint kk = 0;

			while(kk < count) {
				// surfel_t surfel = surfeCache[kk];
				float2 d1 = GetProjectedDistanceFromSurfel(surfacePosition, surfeCache[kk].position, surfeCache[kk].normal);
				float2 d2 = GetProjectedDistanceFromSurfel(surfacePosition, surfeCache[kk+1].position, surfeCache[kk+1].normal);

				float2 times1 = d1 * d1;
				float2 times2 = d2 * d2;

				float exp1 =  exp( times1.x * ( -1.f / (2 * 2 * SURFEL_RADIUS * 2 * SURFEL_RADIUS)) );
				float exp2 = 	exp( times2.x * ( -1.f / (2 * 2 * SURFEL_RADIUS * 2 * SURFEL_RADIUS)) ); 
				
				float div1 = (times1.y * (10.f / (SURFEL_RADIUS)) + 1.f);
				float div2 = (times2.y * (10.f / (SURFEL_RADIUS)) + 1.f);

				float weight1 = exp1 / div1;
				float weight2 = exp2 / div2;

				float dot1 = dot(surfaceNormal, surfeCache[kk].normal);
				float dot2 = dot(surfaceNormal, surfeCache[kk+1].normal);
				float iscovered1 = ( dot1 > .9f ? 1.f : 0.f) * weight1;
				float iscovered2 = ( dot2 > .9f ? 1.f : 0.f) * weight2;

				indirect = mad( surfeCache[kk].indirectLighting, iscovered1, indirect );
				total += weight1;
				
				if(kk+1 < count) {
					indirect = mad( surfeCache[kk+1].indirectLighting, iscovered2, indirect );
					total += weight2;
				}
				kk += 2;
			}
			i+=8;
		}
	}

	return float4(indirect, total);
}


[RootSignature(DeferredLighting_RootSig)]
[numthreads(64, 8, 1)]
void main( uint3 threadId: SV_DispatchThreadID, uint3 groupThreadId: SV_GroupThreadID, uint3 groupId: SV_GroupID )
{
	uint2 pix = threadId.xy;	
	groupOffset = groupId.z;

	localThreadId = groupThreadId;
	uint2 size;
	uTexIndirect.GetDimensions(size.x, size.y);	
	float4 color = ComputeIndirect(pix * 2);

	if(pix.x > size.x || pix.y > size.y) return;

	uTexIndirect[pix] = color + uTexIndirect[pix];

}



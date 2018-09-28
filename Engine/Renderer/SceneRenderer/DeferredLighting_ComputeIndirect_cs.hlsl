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
Texture2D<float4> gTexAO: register(t15);

RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<SurfelBucketInfo> uSurfelBucket: register(u1);
RWTexture2D<float4> uTexIndirect: register(u2);


float4 ComputeIndirect(uint2 pix, uint groupIndex)
{
	float3 surfacePosition = gTexPosition[pix].xyz;
	float3 surfaceNormal = gTexNormal[pix].xyz * 2.f - 1.f;
	float3 surfaceColor = gTexAlbedo[pix].xyz;
	float3 eyePosition;
	{
		float4 eye = mul(inverse(view), mul(inverse(projection), float4(0,0,0,1.f)));
		eyePosition = eye.xyz / eye.w;
	}

	float3 indirect = float3(0,0,0);
	 
	float total = 0;

	uint bucketCount, _;
	uSurfelBucket.GetDimensions(bucketCount, _);

	for(uint q = 0; q < 16; q++) {
		for(uint p = 0; p < 16; p++) {
			SurfelBucketInfo info = uSurfelBucket[GetSpatialHashFromComponent(uint3(p, q, groupIndex))];
	 	
			uint i = info.startIndex;
			while(i < info.startIndex + info.currentCount) {

				float d = distance(surfacePosition, uSurfels[i].position);

				float weight = 1 / ((d*d) / (SURFEL_RADIUS * SURFEL_RADIUS) + SURFEL_RADIUS + 1);
				float iscovered = saturate((dot(surfaceNormal, uSurfels[i].normal))) * weight;
				total += weight;
				indirect =	indirect + 
										(uSurfels[i].indirectLighting) * iscovered;

				i++;
		}
	}
	
	}

	return float4(indirect, total);
}

[RootSignature(DeferredLighting_RootSig)]
[numthreads(32, 32, 1)]
void main( uint3 threadId: SV_DispatchThreadID, uint3 groupId: SV_GroupId )
{
	uint2 pix = threadId.xy;

	uint2 size;
	uTexIndirect.GetDimensions(size.x, size.y);
	
	if(pix.x > size.x || pix.y > size.y) return;

	float4 color = ComputeIndirect(pix * 2, groupId.z);

	uTexIndirect[pix] += color;

}


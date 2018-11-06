#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"
#define SurfelVisual_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 3),UAV(u3, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"


Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<SurfelBucketInfo> uSurfelBucket: register(u1);
RWStructuredBuffer<SurfelHistoryBuffer> uSurfelsHistory: register(u2);
RWTexture2D<float4> uTexSurfelVisual: register(u3);


[RootSignature(SurfelVisual_RootSig)]
[numthreads(32, 32, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex )
{
	uint2 pix = threadId.xy;
	uint2 size;
	gTexNormal.GetDimensions(size.x, size.y);

	if(pix.x >= size.x || pix.y >= size.y) return;
	float2 screen = float2(pix.x, pix.y) / float2(size);

	float3 position, normal;
	float legal;
	{ // get information from the G-Buffer
		position = gTexPosition[pix].xyz;
		legal = gTexPosition[pix].w;
		normal = gTexNormal[pix].xyz * 2.f - float3(1.f, 1.f, 1.f);
	}

	// uTexSurfelVisual[pix] = float4(1.f, 1.f, 1.f, 1.f);

	float blend = length(normal);


	uint bucketcount, _;
	uSurfelBucket.GetDimensions(bucketcount, _);

	float4 color = float4(0.f, 0.f, 0.f, 1.f);
	for(uint j =0; j < bucketcount; j++) {
		SurfelBucketInfo info = uSurfelBucket[j];

		uint count = info.currentCount;
		uint i = info.startIndex;
		while(i < info.startIndex + count) {
			if(isCovered(position, normal, uSurfels[i]) > 0) {
				float4 normalVar = uSurfelsHistory[i].normalizedHSLVariance();
				color =	float4(uSurfelsHistory[i].weightCurve.scale / 5.f, 0 ,0 , 1.f);
				// color =	float4( 0, normalVar.w, 0 , 1.f);
			}
			i++;
		}

	}


	//color /= count;

	uTexSurfelVisual[pix] = blend * color * legal;
}
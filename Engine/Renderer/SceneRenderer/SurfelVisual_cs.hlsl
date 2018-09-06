#include "Common.hlsli"

#define SurfelVisual_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 2),UAV(u2, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"


struct vertex_t {
	float4 position;
};

struct surfel_t {
  float3 position;
  float3 normal;
	float3 color;
};



Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<uint> uNumSurfels: register(u1);
RWTexture2D<float4> uTexSurfelVisual: register(u2);

static const float SURFEL_RADIUS = 1.f;


float isCovered(float3 position, float3 normal, surfel_t surfel) {

	// 1. their normal agree with each other
	float dp = dot(normal, surfel.normal);

	if(dp < 0) return 0;

	// 2. if this is too far, no
	float dist = distance(position, surfel.position);
	if(dist > SURFEL_RADIUS) return 0;

	// 2. project the point to the surfel plane, they are close enough
	float3 projected = surfel.position - dot((position - surfel.position), surfel.normal) * surfel.normal;
	
	return 1.f / (clamp(distance(projected, surfel.position), 0, 1) + 1.f);

}

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
	{ // get information from the G-Buffer
		position = gTexPosition[pix].xyz;
		normal = gTexNormal[pix].xyz * 2.f - float3(1.f, 1.f, 1.f);
	}

	uTexSurfelVisual[pix] = float4(1.f, 1.f, 1.f, 1.f);

	float blend = length(normal);

	uint count = uNumSurfels[0];

	float4 color = float4(0,0,0,0);
	for(uint i = 0; i < count; i++) {
		if(isCovered(position, normal, uSurfels[i]) > 0) {
			color =	float4(uSurfels[i].color, 1.f);
	
		}
			 
	}

	//color /= count;

	uTexSurfelVisual[pix] = blend * color;
}
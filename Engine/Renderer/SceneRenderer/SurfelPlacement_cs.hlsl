#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"
#define SurfelPlacement_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 5, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"



Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<surfel_t> uSurfelsHistory: register(u1);
RWStructuredBuffer<SurfelBucketInfo> uSurfelBucket: register(u2);
RWTexture2D<float4> gTexCoverage: register(u3);
RWTexture2D<float4> uSpawnChance: register(u4);


float coverageAt(uint2 pix) {
	return gTexCoverage[pix].x;
}

struct pixel {
	uint2 coords;
	float coverage;
};


float chanceToSpawnAt(uint2 pix) {
	SCREEN_SIZE_FROM(fullsize, gTexNormal);

	if(pix.x == fullsize.x - 1|| pix.y == fullsize.y - 1) return 0;
	float3 a,b,c,d;

	// a -- b
	// |    |
	// d -- c

	float pixDepth = gTexDepth[pix].x;
	float legal = 0;
	a = gTexPosition[pix].xyz;
	legal = gTexPosition[pix].w;
	b = gTexPosition[pix + uint2(1, 0)].xyz;
	c = gTexPosition[pix + uint2(1, 1)].xyz;
	d = gTexPosition[pix + uint2(0, 1)].xyz;

	float pixArea;
	{
		float3 ad = d - a;
		float3 ab = b - a;
		float3 ac = c - a;

		pixArea = (length(cross(ab, ac)) + length(cross(ac, ad))) * .5f;
	}

	float depthFactor = (1 - pixDepth ) * (1 - pixDepth);
	uSpawnChance[pix] = float4(distance(c, d), distance(d, a), 0, 1.f);
	//pixArea is around 0.004~0.01
	return 8000000.f * depthFactor * pixArea * legal;
	
}

pixel leastCoveredInRange(uint2 topLeft, uint seed) {
	uint2 bottomRight = topLeft + uint2(TILE_SIZE, TILE_SIZE);

	SCREEN_SIZE_FROM(fullsize, gTexNormal);

	bottomRight = clamp(bottomRight, topLeft, fullsize);

	pixel p;
	p.coords = uint2(0,0);
	p.coverage = TILE_SIZE + 1;


	uint2 ij = topLeft;
	Randomf rand = rnd01(seed);
	while(ij.y < bottomRight.y) {
		ij.x = topLeft.x;
		while(ij.x < bottomRight.x) {
			float coverage = coverageAt(ij);

			float chance = chanceToSpawnAt(ij);

			if(coverage <= p.coverage) {
				p.coverage = coverage;
				p.coords = ij; 
			}
			ij.x++;
		}
		ij.y++;
	}
	
	return p;
}

void appendSurfel(surfel_t surfel) {
	uint hash = SpatialHash(surfel.position);

	if(uSurfelBucket[hash].currentCount + uSurfelBucket[hash].startIndex >= uSurfelBucket[hash].endIndex) return; 
	uint offset;
	InterlockedAdd(uSurfelBucket[hash].currentCount, 1, offset);

	uSurfels[uSurfelBucket[hash].startIndex + offset] = surfel;
	uSurfelsHistory[uSurfelBucket[hash].startIndex + offset] = surfel;
}

[RootSignature(SurfelPlacement_RootSig)]
[numthreads(8, 8, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex )
{

	uint2 pixTopLeft = threadId.xy * TILE_SIZE;

	RETURN_IF_OUT_TEX(pixTopLeft, gTexNormal);

	uint uintTime = 0;
	uintTime = asuint(gTime);
	
	uint seed = uintTime * 0x345553 + threadId.x * 0xD2A80A23 + threadId.y * 0x24657ff;
	
	Randomf rand = rnd01(seed);
	rand = rnd01(seed);
	pixel pix = leastCoveredInRange(pixTopLeft, rand.seed);

	if(pix.coverage > 0) return;

	surfel_t surfel;


	float chance = chanceToSpawnAt(pix.coords);

	if(!checkChance(rand, chance)) return;


	float3 color = gTexAlbedo[pix.coords].xyz;
	 /*
	rand = rnd01(rand.seed);
	color.r = rand.value;

	rand = rnd01(rand.seed);
	color.g = rand.value;

	rand = rnd01(rand.seed);
	color.b = rand.value;
		*/
	surfel.position = gTexPosition[pix.coords].xyz;
	surfel.normal =		gTexNormal[pix.coords].xyz * 2.f - float3(1.f, 1.f, 1.f);
	surfel.color = color;
	surfel.indirectLighting = float3(0, 0, 0);
	surfel.age = 0.f;
	surfel.id = -1;
	surfel.__padding = float2(0,0);
	for(uint i = 0; i < TOTAL_HISTORY; i++) {
		surfel.history.buffer[i] = float4(1,1,1,1);
	}
	surfel.history.nextToWrite = 0;

	InitBezierCurve(surfel.weightCurve);
	 /*
	uint hash = SpatialHash(surfel.position);
	uint index = uSurfelBucket[hash].startIndex + uSurfelBucket[hash].currentCount;
	
	uSurfels[index] = surfel;

	InterlockedAdd(uSurfelBucket[hash].currentCount, 1);	 */

	appendSurfel(surfel);
}

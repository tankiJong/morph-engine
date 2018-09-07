#include "Common.hlsli"
#include "Surfel.hlsli"

#define SurfelPlacement_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 4), SRV(t15, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"



Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

AppendStructuredBuffer<surfel_t> uSurfels: register(u0);
RWTexture2D<float4> uSpawnChance: register(u3);
Texture2D<float4> gTexCoverage: register(t15);


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
	a = gTexPosition[pix];
	b = gTexPosition[pix + uint2(1, 0)];
	c = gTexPosition[pix + uint2(1, 1)];
	d = gTexPosition[pix + uint2(0, 1)];

	float pixArea;
	{
		float3 ad = d - a;
		float3 ab = b - a;
		float3 ac = c - a;

		pixArea = (length(cross(ab, ac)) + length(cross(ac, ad))) * .5f;
	}

	float depthFactor = - pixDepth * pixDepth + 2 * pixDepth;
	//pixArea is around 0.004~0.01
	return 1 * pixArea;

	float chance = smoothstep(0, 1, 1.f - pixDepth)
							 * pixArea * (1.f/ ( SURFEL_RADIUS*SURFEL_RADIUS));

	return chance;
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
			uSpawnChance[ij] = float4(chance, chance, chance, 1.f);

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
[RootSignature(SurfelPlacement_RootSig)]
[numthreads(1, 1, 1)]
void main( uint3 threadId : SV_DispatchThreadID )
{

	uint2 pixTopLeft = threadId.xy * TILE_SIZE;

	RETURN_IF_OUT_TEX(pixTopLeft, gTexNormal);

	uint seed = threadId.x * 102400 + threadId.y * 34573645 + uint(gTime*10000);
	
	Randomf rand = rnd01(seed);
	pixel pix = leastCoveredInRange(pixTopLeft, rand.seed);

	if(pix.coverage > .1f) return;

	surfel_t surfel;


	float chance = chanceToSpawnAt(pix.coords);

	if(!checkChance(rand, chance)) return;


	float3 color;

	rand = rnd01(rand.seed);
	color.r = rand.value;

	rand = rnd01(rand.seed);
	color.g = rand.value;

	rand = rnd01(rand.seed);
	color.b = rand.value;

	surfel.position = gTexPosition[pix.coords].xyz;
	surfel.normal =		gTexNormal[pix.coords].xyz * 2.f - float3(1.f, 1.f, 1.f);
	surfel.color = color;

	uSurfels.Append(surfel);
}
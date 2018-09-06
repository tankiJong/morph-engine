#include "Common.hlsli"

#define SurfelPlacement_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"


struct Ray {
	float3 position;
	float3 direction;
};

struct Contact {
	float4 position;
	float3 normal;
	float t;
	bool valid;
};

struct Random {
	uint value;
	uint seed;
};

struct Randomf {
	float value;
	uint seed;
};

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

AppendStructuredBuffer<surfel_t> uSurfels: register(u0);


Random rnd(uint seed)
{
	Random re;
	re.value = seed;

	const uint BIT_NOISE1 = 0xD2A80A23; // 0b1101'0010'1010'1000'0000'1010'0010'0011;
	const uint BIT_NOISE2 = 0xA884F197; // 0b1010'1000'1000'0100'1111'0001'1001'0111;
	const uint BIT_NOISE3 = 0x1B56C4E9; // 0b0001'1011'0101'0110'1100'0100'1110'1001;

	uint mangledBits = seed;
	mangledBits *= BIT_NOISE1;
	mangledBits += seed;
	mangledBits ^= (mangledBits >> 7);
	mangledBits += BIT_NOISE2;
	mangledBits ^= (mangledBits >> 8);
	mangledBits *= BIT_NOISE3;
	mangledBits ^= (mangledBits >> 11);

	re.seed = mangledBits;
	return re;
}

Randomf rnd01(uint seed) {
	Random re = rnd(seed);
	const float MAX_UINT = 4294967296.0;

	Randomf ref;
	ref.seed = re.seed;
	ref.value = (float)re.value / MAX_UINT;

	return ref;
}

Randomf rrange(float min, float max, uint seed) {
	Randomf re = rnd01(seed);

	re.value = re.value * (max - min) + min;
	
	return re;
}


float CoverageOfPixel(float2 screen) {
	float3 position = gTexPosition.SampleLevel(gSampler, screen, 0).xyz;
	float3 normal = gTexNormal.SampleLevel(gSampler, screen, 0).xyz * 2.f - float3(1.f, 1.f, 1.f);
}

[RootSignature(SurfelPlacement_RootSig)]
[numthreads(1, 1, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint2 groupIndex: SV_GroupID )
{
	uint2 pixTopLeft = groupIndex.xy;

	uint seed = threadId.x * 1024 + threadId.y + groupIndex.x + (uint)(gTime*100000);

	uint2 pix = pixTopLeft * 128;

	float3 color;
	

	Randomf r = rrange(0, 128, rnd(seed).seed);
	pix.x += r.value;
	color.r = r.value / 128.f;
	
	r = rrange(0, 128, r.seed);
	pix.y += r.value;
	color.g = r.value / 128.f;

	color.b = rnd01(r.seed).value;

	uint2 size;
	gTexPosition.GetDimensions(size.x, size.y);

	if(pix.x >= size.x || pix.y >= size.y) return;

	float2 screen = float2(pix.x, pix.y) / float2(size);


	surfel_t surfel;

	surfel.position = gTexPosition.SampleLevel(gSampler, screen, 0).xyz;
	surfel.normal = gTexNormal.SampleLevel(gSampler, screen, 0).xyz * 2.f - float3(1.f, 1.f, 1.f);
	surfel.color = color;
	uSurfels.Append(surfel);
}
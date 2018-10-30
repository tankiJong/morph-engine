#include "./Common.hlsli"

#define GenAO_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 7), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"


Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
Texture2D<float4> gTexVelocity: register(t14);

StructuredBuffer<vertex_t> gVerts: register(t15);
Texture2D<float4> gGAO: register(t16);

RWTexture2D<float4> uAO: register(u0);


Contact trace(Ray ray) {
	uint vertCount, stride;
	gVerts.GetDimensions(vertCount, stride);

	Contact contact;
	contact.t = 1e6;
	contact.valid = false;

	for(uint i = 0; i < vertCount; i+=3) {
		Contact c = triIntersection(gVerts[i].position.xyz, gVerts[i+1].position.xyz, gVerts[i+2].position.xyz, gVerts[i].position.w, ray);
		bool valid = c.valid && (c.t < contact.t) && (c.t > 0.f);	 // equal to zero avoid the fail intersaction in the corner	edge
		if(valid)	{
			contact = c;
		}
	}

	return contact;
}

static uint seed;

float4 samplePreviousAO(uint2 pix, float2 screen, float3 normal, float3 position) {

	float3 shift = gTexVelocity[pix].xyz * gTime;

	float4 currentView = mul(view, float4(position, 1.f));
	currentView /= currentView.w;

	// float2 currentNdc = float2(screen.x, 1.f - screen.y) * 2.f - 1.f;	// [-1. 1]
	// float2 prevNdc = currentNdc - shift;
	float4 prevView = currentView - float4(shift, 0.f);
	float4 prevProj = mul(prev_projection, prevView);
	prevProj /= prevProj.w;
	


	float2 prevNdc = prevProj.xy;

	bool2 notEqual = clamp(prevNdc, float2(-1, -1), float2(1,1)) != prevNdc;
	if(notEqual.x || notEqual.y) return float4(0.f, 0.f, 0.f, 0.f);

	float2 prevscreen = prevNdc * .5f + .5f; // [0, 1]
	prevscreen.y = 1.f - prevscreen.y;
	
	uint2 size;
	uAO.GetDimensions(size.x, size.y);

	uint2 prevPix = uint2(prevscreen.x * float(size.x), prevscreen.y * float(size.y));

	float4 ssample = gGAO[prevPix];
	float oldDepth = ssample.z;

	// if(isnan(oldDepth)) return float4(10000, prevProj.z / 2.f + .5f, oldDepth, 0.f);
	// if(distance(oldDepth, prevProj.z / 2.f + .5f) > .001f && gFrameCount > 1) return float4(oldDepth, prevProj.z / 2.f + .5f, oldDepth, 0.f);

	const float Z_TEST_BIAS = 0.001f;
	float weight;
	if(distance(oldDepth, prevProj.z / 2.f + .5f) > Z_TEST_BIAS && gFrameCount > 1) {
		weight = 1.f;
	} else {
		weight = ssample.w;
	}
	return float4(ssample.xyz, weight); 
	// return float4(1,1,1,1);
}


float3 ndc(float3 world) {
	float4 proj = mul(mul(projection, view), float4(world, 1.f));

	proj = proj / proj.w;

	proj.z = proj.z / 2.f + .5f;

	return proj.xyz;
}

[RootSignature(GenAO_RootSig)]
[numthreads(16, 16, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex )
{
	uint2 pix = threadId.xy;
	uint2 size;
	uAO.GetDimensions(size.x, size.y);

	if(outScreen(pix, size)) return;
	float2 screen = float2(pix.x, pix.y) / float2(size);


	float3 position, normal;
	{ // get information from the G-Buffer
		position = gTexPosition[pix].xyz;
		normal = gTexNormal[pix].xyz * 2.f - 1.f;
	}
	
	if( length(normal) < 0.0001f ) return;
	float occlusion = 0.f;

	seed = threadId.x * 102467 + threadId.y * 346755 + groupIndex + uint(gTime*10000);

	float hitDistance = 0;
	
	bool occluded = false; 
	{
		Ray ray = GenReflectionRay(seed, float4(position, 1.f), normal);
	
		Contact c = trace(ray);

		occluded = c.valid;
	
		float occ = lerp(0, 1, (float)occluded);
		hitDistance += c.t;
		occlusion += occ;
	}
	 		 
	occlusion = occlusion / ((hitDistance * hitDistance) + 1);

	occlusion = 1.f - occlusion;


	float2 color = float2(occlusion, hitDistance);

	// uAO[threadId.xy] = float4(color, hitDistance);

	float depth = ndc(position).z;
	float4 previousAO = samplePreviousAO(pix, screen, normal, position);
	// uAO[threadId.xy] = float4(previousAO.x, previousAO.y, depth, previousAO.w);
	// return;
	float2 ao = ( previousAO.xy * previousAO.w + color ) / (previousAO.w + 1);

	ao.y = occluded ? ao.y : previousAO.y;
	float4 result = float4(ao, depth, clamp(previousAO.w + 1, 0, 256));
	uAO[threadId.xy] = result;
	// uAO[threadId.xy] = float4(1, 1, 1, 1.f);
}
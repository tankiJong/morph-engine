#include "./Common.hlsli"

#define GenAO_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"


Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

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


	float occlusion = 0.f;

	seed = threadId.x * 102467 + threadId.y * 346755 + groupIndex + uint(gTime*10000);

	float hitDistance = 0;
	
	{
		Ray ray = GenReflectionRay(seed, float4(position, 1.f), normal);	
		Contact c = trace(ray);
	
		bool occluded = c.valid;	
		occlusion += lerp(0, 1, (float)occluded);
		hitDistance += c.t;	
	}
	 		 
	occlusion = 1.f - occlusion;

	float3 color = float3(occlusion, occlusion, occlusion);

	uAO[threadId.xy] = float4(color, hitDistance);
	// uAO[threadId.xy] = float4(1, 1, 1, 1.f);
}
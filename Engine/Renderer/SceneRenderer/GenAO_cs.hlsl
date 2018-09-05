#include "./Common.hlsli"

#define GenAO_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"

struct vertex_t {
	float4 position;
};

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


Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

RWTexture2D<float4> uAO: register(u0);

Ray GenPrimaryRay(float3 ndc) {
	Ray ray;
	
	float4x4 invView = inverse(view);
	float4 _worldCoords = mul(invView, mul(inverse(projection), float4(ndc, 1.f)));
	float3 worldCoords = _worldCoords.xyz / _worldCoords.w;

	ray.position = worldCoords;

	float3 origin = mul(invView, float4(0, 0, 0, 1.f)).xyz;
	ray.direction = normalize(worldCoords - origin);

	return ray;
}

Contact triIntersection(float3 a, float3 b, float3 c, float color, Ray ray) {

	Contact contact;

	float3 ab = b - a;
	float3 ac = c - a;
	float3 normal = normalize(cross(ac, ab));
	contact.normal = normal;

	contact.valid = dot(normal, ray.direction) < 0;

	float t = (dot(a - ray.position, normal)) / dot(normal, ray.direction);
	contact.t = t;
	contact.position.xyz = ray.position + t * ray.direction;
	contact.position.w = color;

	float3 p = contact.position.xyz;
	contact.valid = contact.valid && dot(cross(p - a, b - a), normal) >= 0;
	contact.valid = contact.valid && dot(cross(p - b, c - b), normal) >= 0;
	contact.valid = contact.valid && dot(cross(p - c, a - c), normal) >= 0;

	return contact;
}




Random rnd(uint seed)
{
	Random re;
	re.value = seed;

	seed ^= (seed << 13);
  seed ^= (seed >> 17);
  seed ^= (seed << 5);

	re.seed = seed;
	return re;
}

Ray GenShadowRay(inout uint seed, float4 position, float3 normal) {
	float MAX_UINT = 4294967296.0;

	float3 direction = normal;

	Random r = rnd(seed);
	seed = r.seed; 
	float x = float(r.value) * (1.0f / MAX_UINT)- .5f;
	 
	r = rnd(seed);
	seed = r.seed;
	float y = float(r.value) * (1.0f / MAX_UINT)- .5f;

	r = rnd(seed);
	seed = r.seed;
	float z = float(r.value) * (1.0f / MAX_UINT)- .5f;
						
	float3 right = float3(x,y,z);
	float3 _tan = normalize(right);
	float3 bitan = cross(_tan, normal);
	float3 tan = cross(bitan, normal);

	float3x3 tbn = transpose(float3x3(tan, normal, bitan));

	r = rnd(seed);
	seed = r.seed;
	float a = abs(float(r.value) * (1.0f / MAX_UINT) - .5f);

	r = rnd(seed);
	seed = r.seed;
	float b = float(r.value) * (1.0f / MAX_UINT);

	r = rnd(seed);
	seed = r.seed;
	float c = float(r.value) * (1.0f / MAX_UINT) - .5f;
	
	float3 sample = normalize(mul(tbn, float3(a, b, c))) + normal * 0.0001f;

	Ray ray;

	ray.direction = sample;
	ray.position = position.xyz;

	return ray;
}

Contact trace(Ray ray) {
	uint vertCount, stride;
	gVerts.GetDimensions(vertCount, stride);

	Contact contact;
	contact.t = 1e6;
	contact.valid = false;

	for(uint i = 0; i < vertCount; i+=3) {
		Contact c = triIntersection(gVerts[i].position.xyz, gVerts[i+1].position.xyz, gVerts[i+2].position.xyz, gVerts[i].position.w, ray);
		bool valid = c.valid && (c.t < contact.t) && (c.t >= 0.f);	 // equal to zero avoid the fail intersaction in the corner	edge
		if(valid)	{
			contact = c;
		}
	}

	return contact;
}

static uint SSeed;


[RootSignature(GenAO_RootSig)]
[numthreads(32, 32, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex )
{
	uint2 pix = threadId.xy;
	uint2 size;
	uAO.GetDimensions(size.x, size.y);

	if(pix.x >= size.x || pix.y >= size.y) return;
	float2 screen = float2(pix.x, pix.y) / float2(size);


	float3 position, normal;
	{ // get information from the G-Buffer
		position = gTexPosition.SampleLevel(gSampler, screen, 0).xyz;
		normal = gTexNormal.SampleLevel(gSampler, screen, 0).xyz * 2.f - float3(1.f, 1.f, 1.f);
	}


	float occlusion = 0.f;

	float seed = threadId.x * 1024 + threadId.y + groupIndex + gTime*1000;

	for(uint i = 0; i < 16; i++) {
		Ray ray = GenShadowRay(seed, float4(position, 1.f), normal);
		//ray.direction = float3(-0.5f, 0.5f, 0.f);
		/*
		if(dot(ray.direction, normal) > 0 ) {
			uAO[threadId.xy] = float4(1, 1, 1, 1.f);
		}	else {
			uAO[threadId.xy] = float4(0, 0, 0, 1.f);
		}
		 */
		Contact c = trace(ray);

		bool occluded = c.valid;
		if(occluded) {
			// Output[threadId.xy] = float4(contact.position.w,contact.position.w,contact.position.w, 1.f);

			occlusion+= 1.f / (dot(normal, ray.direction) * c.t + 1.f);
		}
	}
	 		 
	occlusion = occlusion / 16.f;
	occlusion = 1.f - occlusion;

	float3 color = float3(occlusion, occlusion, occlusion);
	uAO[threadId.xy] = float4(color, 1.f);
}
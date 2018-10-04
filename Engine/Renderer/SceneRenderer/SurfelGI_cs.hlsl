#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"

#define SurfelGI_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 3), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"

Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<uint> uNumSurfels: register(u1);
RWStructuredBuffer<SurfelBucketInfo> uSurfelBucket: register(u2);



static uint seed;


bool GetSurfelAt(float3 position, float3 normal, out float3 indirect) {

	float maxCovered = 0.f;
	float covered = 0;
	float3 color = float3(0, 0, 0);

	uint hash = SpatialHash(position);

	// uint bucketCount, _;
	// uSurfelBucket.GetDimensions(bucketCount, _);

	SurfelBucketInfo info = uSurfelBucket[hash];
	uint i= info.startIndex;
	while(i < info.startIndex + info.currentCount ) {
		float factor = isCovered(position, normal, uSurfels[i]);
		covered += factor;
		color += factor * uSurfels[i].indirectLighting;
		i++;
	}
	
	indirect = color / covered;

	// seems covered is always 0
	return covered > 0.f;
}

// if false, stop tracing
bool russianRoulette(inout float weight) {
	static const float STOP_THREHOLD = .5f;
	static const float STOP_CHANCE = .1f;

	if(weight > STOP_THREHOLD) return true;

	Randomf roll = rnd01(seed);
	seed = roll.seed;
	if(roll.value < STOP_CHANCE) return false;

	weight = weight / ( 1 - STOP_CHANCE);
}

float3 PathTracing(Ray startRay, float3 startPosition, float3 startNormal, float3 startColor) {
	Ray ray = startRay;
	uint bounce = 0;

	float3	colors[10];
	float3 diffuses[10];
	float4 totals[10];
	diffuses[0] = float3(0,0,0); // I only want the indirect part
	colors[0] =	startColor;

	for(uint xx = 0; xx < 3; xx++) {
		bounce++;
		uint vertCount, stride;
		gVerts.GetDimensions(vertCount, stride);

		Contact contact;
		contact.t = 1e6;
		contact.valid = false;

		float3 albedo = float3(0,0,0);
		for(uint i = 0; i < vertCount; i+=3) {
			Contact c = triIntersection(gVerts[i].position.xyz, gVerts[i+1].position.xyz, gVerts[i+2].position.xyz, gVerts[i].position.w, ray);
			bool valid = c.valid && (c.t < contact.t) && (c.t >= 0.f);	 // equal to zero avoid the fail intersaction in the corner	edge
			if(valid)	{
				contact = c;
				albedo = gVerts[i].color.xyz;
			}
		}

		colors[bounce] = albedo;

		totals[bounce].w = contact.t;
		if(!contact.valid) {
			totals[bounce] = float4(0, 0, 0, 0);
			break;
		} else {
			float3 diffuse = Diffuse(contact.position.xyz, contact.normal, float3(1, 1, 1), gLight);
			diffuses[bounce] = diffuse;

			float3 indirect;
			bool hit = GetSurfelAt(contact.position.xyz, contact.normal, indirect);
			if(hit) {
				totals[bounce].xyz = 
					(indirect * 2 + diffuse / 3.141592653f);
				break;
			}	else {
				totals[bounce].xyz = diffuse / 3.141592653f;
			}

		}

		ray = GenReflectionRay(seed, contact.position, contact.normal);
	}
	
	bounce++;
	totals[bounce] = float4(0, 0, 0, 0.f);

	for(uint i = bounce; i > 0; i--) {

		float t = totals[i].w;
		float3 direct = diffuses[i - 1].xyz;
		float3 indirect = totals[i].xyz * colors[i];
		
		totals[i - 1].xyz
			= (indirect * 2 + diffuses[i-1] / 3.141592653f);
	}

	return totals[0].xyz;								// this give us the 'energy' of A ray
}

void updateSurfels(inout surfel_t surfel) {
	float age = surfel.age;

	uint index = surfel.nextToWrite;
	float4 history[TOTAL_HISTORY] = surfel.history;
	if(age < TOTAL_HISTORY) {
		Ray ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		float3 indirect = PathTracing(ray, surfel.position, surfel.normal, surfel.color) * dot(ray.direction, surfel.normal); 
		ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		indirect += PathTracing(ray, surfel.position, surfel.normal, surfel.color) * dot(ray.direction, surfel.normal); 
		ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		indirect += PathTracing(ray, surfel.position, surfel.normal, surfel.color) * dot(ray.direction, surfel.normal);
		ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		indirect += PathTracing(ray, surfel.position, surfel.normal, surfel.color) * dot(ray.direction, surfel.normal);
		ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		indirect += PathTracing(ray, surfel.position, surfel.normal, surfel.color) * dot(ray.direction, surfel.normal);
		
		indirect = indirect / 5.f;
		history[index] = float4((surfel.indirectLighting * age + indirect) / (age + 1.f), 0);

		float3 lighting = float3(0,0,0);
		for(uint i = 0; i <= index; i++) {
			lighting += history[i].xyz;
		}

		float3 average = lighting / float(index + 1);

		float variance = float3(0,0,0);
		for(uint i = 0; i < index; i++) {
			variance += distance(average, history[i].xyz) * distance(average, history[i].xyz);
		}

		variance = abs(variance / float(index + 1));

		float k = variance;

		surfel.indirectLighting = lerp(average, surfel.indirectLighting, 1.f /(k + 1));
		// history[index] = float4(surfel.indirectLighting, 0);
	} else {
		Ray ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		// surfel.indirectLighting = PathTracing(ray, surfel.position, surfel.normal, surfel.color);
		float3 newIndirect = PathTracing(ray, surfel.position, surfel.normal, surfel.color) * dot(ray.direction, surfel.normal);;

		history[index] = float4(newIndirect, 0.f);

		float3 lighting = float3(0,0,0);
		for(uint i = 0; i < TOTAL_HISTORY; i++) {
			lighting += history[i].xyz;
		}
		
		float3 average = lighting / TOTAL_HISTORY;

		float variance = float3(0,0,0);
		for(uint i = 0; i < 16; i++) {
			variance += distance(average, history[i].xyz);
		}

		variance = abs(variance) / TOTAL_HISTORY;

		float k = variance * distance(surfel.indirectLighting, average);

		surfel.indirectLighting = lerp(average, surfel.indirectLighting, 1.f /(k + 1));
		// history[index] = float4(surfel.indirectLighting, 0.f);
		// surfel.indirectLighting = lerp(surfel.indirectLighting, newIndirect, 1/(age + 1));
	}

	surfel.nextToWrite = (surfel.nextToWrite + 1) % TOTAL_HISTORY;
	surfel.history = history;
	surfel.age++;
}

[RootSignature(SurfelGI_RootSig)]
[numthreads(16, 16, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex, uint3 groupId: SV_GroupId )
{
	seed = threadId.x * 10000 + threadId.y * 121144 + gTime * 367860;

	// for(uint j = 0; j < hash; j++) {
		SurfelBucketInfo info = uSurfelBucket[16*16*groupId.z + threadId.x * 16 + threadId.y];

		uint i = info.startIndex;
		while(i < info.startIndex + info.currentCount) {
			if(uSurfels[i].age == 1) {
				uSurfels[i].id = i;
			}

			updateSurfels(uSurfels[i]);
			i++;
		}

}
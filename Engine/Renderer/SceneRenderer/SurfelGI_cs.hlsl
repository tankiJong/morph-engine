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

float3 computeDiffuse(float3 surfacePosition, float3 surfaceNormal) {
	// return float3(0,0,0);
	Ray ray;

	float maxDist = length(gLight.position - surfacePosition);
	ray.direction = normalize(gLight.position - surfacePosition);

	ray.position = surfacePosition + surfaceNormal * 0.0001f;

	Contact c = trace(ray);


	if(c.valid && c.t < maxDist) return float3(0,0,0);
	
	return Diffuse(surfacePosition, surfaceNormal, float3(1, 1, 1), gLight);

}

bool GetSurfelAt(float3 position, float3 normal, inout float3 indirect) {

	float maxCovered = 0.f;
	float covered = 0;

	uint hash = SpatialHash(position);

	// uint bucketCount, _;
	// uSurfelBucket.GetDimensions(bucketCount, _);

	SurfelBucketInfo info = uSurfelBucket[hash];
	uint i= info.startIndex;
	bool hit = false;

	float maxFactor = 0.f;
	while(i < info.startIndex + info.currentCount ) {
		float factor = isCovered(position, normal, uSurfels[i]);
		hit = hit || factor > .95f;
		if(factor > maxFactor) {
			indirect = uSurfels[i].indirectLighting;
			maxFactor = factor;
		}
		i++;
	}
	
	return hit;
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

	float3	colors[20];
	float3 diffuses[20];
	float4 totals[20];
	float dots[20];
	diffuses[0] = float3(0,0,0); // I only want the indirect part
	colors[0] =	startColor;
	dots[0] = saturate(dot(startRay.direction, startNormal));

	for(uint xx = 0; xx < 2; xx++) {
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

		totals[bounce].w = contact.t;
		dots[bounce] = 0;

		if(!contact.valid) {
			diffuses[bounce] = float3(0, 0, 0);
			totals[bounce] = float4(0, 0, 0, 0);
			colors[bounce] = albedo;
			break;
		} else {
			float3 diffuse = computeDiffuse(contact.position.xyz, contact.normal);
			diffuses[bounce] = diffuse;
			colors[bounce] = albedo;
			 
			float3 indirect;
			bool hit = GetSurfelAt(contact.position.xyz, contact.normal, indirect);
			if(hit) {
				totals[bounce].xyz = 
					(indirect * 2.f * M_PI + diffuse) * albedo / M_PI;
				break;
			}	

		}

		ray = GenReflectionRay(seed, contact.position, contact.normal);
		dots[bounce] = saturate(dot(ray.direction, contact.normal));
	}

	for(int i = bounce; i >= 1; i--) {
		float3 indirect = totals[i].xyz * dots[i - 1] * 2.f * M_PI;
		
		totals[i - 1].xyz
			= (indirect + diffuses[i - 1]) * colors[i - 1] / M_PI;
	}
	
	float3 indirect = totals[1].xyz * dots[0];
	
	return indirect;
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
		// history[index] = float4((surfel.indirectLighting * age + indirect) / (age + 1.f), 0);
		history[index] = float4(indirect, 0.f);

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
		// surfel.indirectLighting = PathTracing(ray, surfel.position, surfel.normal, surfel.color);
		float variance = 0;
		{
			float3 lighting = float3(0,0,0);
			for(uint i = 0; i < TOTAL_HISTORY; i++) {
				lighting += history[i].xyz;
			}
		
			float3 average = lighting / TOTAL_HISTORY;

			for(uint i = 0; i < 16; i++) {
				variance += distance(average, history[i].xyz);
			}

			variance = abs(variance) / TOTAL_HISTORY;
		}

		Ray ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		float3 newIndirect = PathTracing(ray, surfel.position, surfel.normal, surfel.color) * dot(ray.direction, surfel.normal);;

		history[index] = float4(newIndirect, 0.f);

		float newVariance = 0;

		float3 lighting = float3(0,0,0);
		for(uint i = 0; i < TOTAL_HISTORY; i++) {
			lighting += history[i].xyz;
		}
		float3 average = lighting / TOTAL_HISTORY;
		

		for(uint i = 0; i < 16; i++) {
			newVariance += distance(average, history[i].xyz);
		}

		newVariance = abs(newVariance) / TOTAL_HISTORY;

		float k = distance(surfel.indirectLighting, average);

		k = ( 1 + saturate(abs(newVariance - variance))) / (k + 1.f);
		surfel.indirectLighting = surfel.indirectLighting * (1 - k) + average * k;
		history[index] = float4(surfel.indirectLighting, 0.f);
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
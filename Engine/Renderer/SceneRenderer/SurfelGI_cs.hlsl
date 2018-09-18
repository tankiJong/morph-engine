#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"

#define SurfelGI_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 5), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 2),UAV(u2, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"

Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<uint> uNumSurfels: register(u1);
RWTexture2D<float4> uTexSurfelVisual: register(u2);



static uint seed;


bool GetSurfelAt(float3 position, float3 normal, out float3 indirect) {


	float maxCovered = 0.f;
	float covered = 0;
	float3 color = float3(0,0,0);
	for(uint i = 0; i < uNumSurfels[0]; i++) {
		float factor = isCovered(position, normal, uSurfels[i]);
		covered += factor;
		color += factor * uSurfels[i].indirectLighting;
	}
	
	indirect = color / covered;
	
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

float3 PathTracing(Ray startRay) {
	Ray ray = startRay;
	uint bounce = 0;
	float final = float3(0, 0, 0);

	float4	color[10];
	color[0].xyz = float3(0, 0, 0);
	while(bounce < 5) {
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
				albedo = gVerts[i].color;
			}
		}

		color[bounce].w = contact.t;

		if(!contact.valid) {
			color[bounce] = float4(0, 0, 0, 0);
			break;
		} else {
			float3 diffuse = Diffuse(contact.position.xyz, contact.normal, albedo, gLight);

			float3 indirect;
			bool hit = GetSurfelAt(contact.position.xyz, contact.normal, indirect);
			if(hit) {
				float BRDF = 1.f / 3.1415926f; // hard coded for now, energy conservation
				color[bounce].xyz = 
					Attenuation(1.f, contact.t, float3(1.f, 1.f, 1.f)) * (indirect * BRDF) + diffuse;
				break;
			}	else {
				color[bounce].xyz = diffuse;
			}

		}

		ray = GenReflectionRay(seed, contact.position, contact.normal);
	}
	
	bounce++;
	color[bounce] = float4(0,0,0,0);

	for(uint i = bounce; i > 0; i--) {

		float t = color[i - 1].w;
		float3 direct = color[i-1].xyz;
		float3 reflected = color[i].xyz;
		float BRDF = 1.f / 3.1415926f; // hard coded for now, energy conservation
		
		color[i - 1].xyz
			= Attenuation(1.f, t, float3(1.f, 1.f, 1.f)) * (reflected * BRDF) + direct;
	}

	return color[0].xyz;
}

void updateSurfels(inout surfel_t surfel) {
	float age = surfel.age;
	/*if(age < 4) {
		Ray ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		float3 indirect = PathTracing(ray); 
		ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		indirect += PathTracing(ray); 
		ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		indirect += PathTracing(ray);
		ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		indirect += PathTracing(ray);
		ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		indirect += PathTracing(ray);
		
		surfel.indirectLighting = (surfel.indirectLighting * age + indirect) / (age + 5.f);
	} else {		*/
		Ray ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		surfel.indirectLighting = PathTracing(ray);
		// surfel.indirectLighting = lerp(surfel.indirectLighting, PathTracing(ray), 1/(age + 1));
	//}


	surfel.age++;
}

[RootSignature(SurfelGI_RootSig)]
[numthreads(1, 1, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex )
{
	seed = threadId.x * 10000121 + threadId.y * 121144362 + gTime * 367868766;

	for(uint i = 0; i < uNumSurfels[0]; i++) {
		if(uSurfels[i].age == 1) {
			uSurfels[i].id = i;
		}
		updateSurfels(uSurfels[i]);

	}
}
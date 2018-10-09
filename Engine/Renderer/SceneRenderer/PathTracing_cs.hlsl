#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"

#define PathTracing_RootSig \
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

RWTexture2D<float4> uTexScene: register(u0);



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
	for(uint xx = 0; xx < 16; xx++) {
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

		float3 lightDir = gLight.position - ray.position;
		float lightLen = length(lightDir);
		bool hitLight = dot(lightDir, ray.direction)/lightLen == 1.f && lightLen < contact.t;
		if(hitLight) {
			diffuses[bounce] = float3(gLight.color.w, gLight.color.w, gLight.color.w);
			totals[bounce].xyz = diffuses[bounce] / (2 * 3.1415926f);
			colors[bounce] = gLight.color.xyz;
			break;
		} 

		if(!contact.valid) {
			diffuses[bounce] = float3(0, 0, 0);
			totals[bounce] = float4(0, 0, 0, 0);
			colors[bounce] = albedo;
			break;
		}	else {
			diffuses[bounce] = computeDiffuse(contact.position.xyz, contact.normal);
			colors[bounce] = albedo;
		}

		ray = GenReflectionRay(seed, contact.position, contact.normal);
		dots[bounce] = saturate(dot(ray.direction, contact.normal));
	}

	bounce++;
	totals[bounce] = float4(0, 0, 0, 0.f);
	colors[bounce] = float3(0, 0, 0);
	for(int i = bounce; i >= 1; i--) {
		float3 indirect = totals[i].xyz * dots[i - 1] * 2 * 3.141592f;
		
		totals[i - 1].xyz
			= (indirect + diffuses[i-1]) * colors[i - 1] / 3.1415926f;
	}
	
	float3 indirect = totals[1].xyz * dots[0];
	
	return indirect;
}

[RootSignature(PathTracing_RootSig)]
[numthreads(16, 16, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex, uint3 groupId: SV_GroupId )
{
	seed = threadId.x * 10000 + threadId.y * 121144 + gTime * 367860;

	uint2 pix = threadId.xy;

	uint2 size;
	uTexScene.GetDimensions(size.x, size.y);
	
	if(pix.x > size.x || pix.y > size.y) return;

	float4 position = gTexPosition[pix];
	float3 normal = gTexNormal[pix].xyz * 2.f - 1.f;
	float3 color = gTexAlbedo[pix].xyz;


	float3 diffuse = computeDiffuse(position.xyz, normal);
	float3 indirect = 0;
	for(uint i = 0; i < 16; i++) {
		Ray ray = GenReflectionRay(seed, position, normal);
		indirect += PathTracing(ray, position.xyz, normal, color);

	}

	indirect /= (16.f / (2 * 3.1415926f));

	// float3 indirect = float3(0,0,0);
	float3 finalColor = ( diffuse + indirect ) * color / 3.14159f;

	const float GAMMA = 1.f / 2.1;
	finalColor =  pow(finalColor, float3(GAMMA, GAMMA, GAMMA));
	finalColor = saturate(finalColor);

	finalColor = (uTexScene[pix].xyz * gFrameCount + finalColor)	/ (gFrameCount + 1);

	uTexScene[pix] = float4(finalColor, 1.f);
}
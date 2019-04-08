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
StructuredBuffer<Prim> gVerts:	register(t13);
StructuredBuffer<BVHNode> gBvh: register(t14);

RWTexture2D<float4> uTexScene: register(u0);

static uint seed;
float3 computeDiffuse(float3 surfacePosition, float3 surfaceNormal) {
	// return float3(0,0,0);
	Ray ray;

	float maxDist = length(gLight.position - surfacePosition);
	ray.direction = normalize(gLight.position - surfacePosition);

	ray.position = surfacePosition + surfaceNormal * 0.0001f;

	// uint count, _;
	// gVerts.GetDimensions(count, _);
	// Contact c;
	// c.t = 1e6;
	// c.valid = false;
	// for(uint i = 0; i < count; i++) {
	// 	Prim p = gVerts[i];
	// 	Contact contact = triIntersection(p.p[0], p.p[1], p.p[2], 1, ray);
	// 	if(contact.valid && (contact.t < c.t) && (contact.t > 0.f)) {
	// 		c = contact;
	// 	}
	// }
	Contact c = trace(ray, gBvh, gVerts);

	Randomf r = rnd01(seed);
	seed = r.seed; 
	float x = (r.value - .5f)*2.f;
						
	float3 right = float3(x,1,1);
	float3 _tan = normalize(right);
	float3 bitan = normalize(cross(_tan, surfaceNormal));
	float3 tan = cross(bitan, surfaceNormal);

	if(c.valid && c.t < maxDist) return float3(0,0,0);
	
	float4 camPosition = mul(inverse(view), 0.f.xxxx);
	camPosition = camPosition / camPosition.w;

	float3 diffuse, specular;
	return pbrDirectLighting(1.f.xxx, gRoughness, gMetallic, surfacePosition, camPosition.xyz, surfaceNormal, tan, bitan, gLight,	diffuse, specular);
	return diffuse;
	return Diffuse(surfacePosition, surfaceNormal, gLight.color.xyz, gLight);

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
	uint xx  = 0;
	for(; xx < 3; xx++) {
		bounce++;
		uint vertCount, stride;
		gVerts.GetDimensions(vertCount, stride);

		float4 albedo = float4(0, 0, 0, 1);

		// uint count, _;
		// gVerts.GetDimensions(count, _);
		// Contact contact;
		// contact.t = 1e6;
		// contact.valid = false;
		// for(uint i = 0; i < count; i++) {
		// 	Prim p = gVerts[i];
		// 	Contact c = triIntersection(p.p[0], p.p[1], p.p[2], 1, ray);
		// 	if(c.valid && (c.t < contact.t) && (c.t > 0.f)) {
		// 		contact = c;
		// 		albedo = p.c[0];
		// 	}
		// }
		Contact contact = trace(ray, gBvh, gVerts, albedo);


		totals[bounce].w = contact.t;
		dots[bounce] = 0;
 
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
		float3 indirect = totals[i].xyz * dots[i - 1] * 2 * M_PI;
		
		totals[i - 1].xyz
			= (indirect + diffuses[i-1]) * colors[i - 1] / M_PI;
	}
	
	float3 indirect = totals[1].xyz * dots[0];
	
	return indirect;
}

[RootSignature(PathTracing_RootSig)]
[numthreads(16, 16, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex, uint3 groupId: SV_GroupId )
{
	seed = threadId.x * 18600 + threadId.y * 121144 + gFrameCount;	


	uint2 pix = threadId.xy;
	

	uint2 size;
	uTexScene.GetDimensions(size.x, size.y);
	
	if(pix.x > size.x || pix.y > size.y) return;

	// float3 ndc = float3(float2(pix.x, size.y - pix.y) / float2(size) * 2.f - 1.f, -1.f);

	float4 position = gTexPosition[pix];
	float3 normal = gTexNormal[pix].xyz * 2.f - 1.f;
	float3 color = gTexAlbedo[pix].xyz;


	float3 indirect = 0;
	float3 direction;

	const uint SPP = 8;
	for(uint i = 0; i < SPP; i++) {
		Ray ray = GenReflectionRay(seed, position, normal);
		// direction += ray.direction;
		indirect += PathTracing(ray, position.xyz, normal, color);

	}

	float3 diffuse = computeDiffuse(position.xyz, normal);
	// direction /= 8;
	// uTexScene[pix] = (uTexScene[pix] *gFrameCount + float4(direction * .5f + .5f, 1.f))	/ (gFrameCount + 1);
	// return;

	indirect /= (float(SPP) / (2 * M_PI));

	// float3 indirect = float3(0,0,0);
	float3 finalColor = ( indirect + diffuse ) * color / M_PI;

	const float GAMMA = 1.f / 2.1;
	finalColor =  pow(finalColor, float3(GAMMA, GAMMA, GAMMA));
	finalColor = saturate(finalColor);

	finalColor = (uTexScene[pix].xyz * gFrameCount + finalColor)	/ (gFrameCount + 1);
	
	uTexScene[pix] = float4(finalColor, 1.f);
	// uTexScene[pix] = float4(float(triangleIntersetionCalledTimes) / 100, 0, 0, 1.f);
	//uTexScene[pix] = float4(float(boxHit) / 64, 0, 0, 1.f);
}
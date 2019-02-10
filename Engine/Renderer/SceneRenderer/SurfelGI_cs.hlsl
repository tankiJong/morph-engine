#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"

#define SurfelGI_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 6), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 4, flags = DESCRIPTORS_VOLATILE), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"

Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<Prim> gVerts:	register(t14);
StructuredBuffer<BVHNode> gBvh: register(t15);

RWStructuredBuffer<surfel_t> uSurfels: register(u0);
RWStructuredBuffer<surfel_t> uSurfelsPreviousFrame: register(u1);
RWStructuredBuffer<SurfelBucketInfo> uSurfelBucket: register(u2);
RWStructuredBuffer<SurfelHistoryBuffer> uSurfelsHistory: register(u3);



static uint seed;
Contact trace(Ray ray) {
	uint vertCount, stride;
	gVerts.GetDimensions(vertCount, stride);

	Contact contact = trace(ray, gBvh, gVerts);

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
	return Diffuse(surfacePosition, surfaceNormal, 1.0f.xxx, gLight);

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

	indirect = float3(0,0,0);
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


float3 PathTracing(Ray startRay, float3 startPosition, float3 startNormal, float3 startColor) {
	Ray ray = startRay;
	uint bounce = 0;

	float3	colors[10];
	float3 diffuses[10];
	float4 totals[10];
	float dots[10];

	diffuses[0] = float3(0,0,0); // I only want the indirect part
	colors[0] =	startColor;
	dots[0] = saturate(dot(startRay.direction, startNormal));

	for(uint xx = 0; xx < 1; xx++) {
		bounce++;
		uint vertCount, stride;
		gVerts.GetDimensions(vertCount, stride);


		float4 albedo;
		
		Contact contact = trace(ray, gBvh, gVerts, albedo);

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

			GetSurfelAt(contact.position.xyz, contact.normal, indirect);
			totals[bounce].xyz = 
				(indirect * 2.f * M_PI + diffuse) * albedo.xyz / M_PI;

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

float3 adaptiveAverage(float3 original, float3 ssample, float variance) {
	float diff =  abs(variance);
	// Debug.Log("Original: " + original + "|Diff: " + diff);
	float k = diff;
	
	k = k * .001f + 0.0005f;
	// k = lerp(1.f / (1.f * 1024.f), 1.f / 8.f, diff);
	// k = k * k * k;
	// k = smoothstep(0, 0.9f, k);
	return (original * (1 - k) + ssample * k);
}


void updateSurfels(inout surfel_t surfel, inout SurfelHistoryBuffer history) {
	float age = surfel.age;
	
	float3 indirect = float3(0,0,0);

	{
		Ray ray = GenReflectionRay(seed, float4(surfel.position, 1.f), surfel.normal);
		indirect = PathTracing(ray, surfel.position, surfel.normal, surfel.color) * dot(ray.direction, surfel.normal);;
	}
	float4 normalizedVariance = history.normalizedHSLVariance();
	history.write(indirect, normalizedVariance.z);
	float4 variance = abs(history.variance());

	// normalizedVariance = lerp(0, 1, clamp(abs(normalizedVariance - float2(.3f, .3f)), 0 , 1));
	float range = .0f;
	normalizedVariance = clamp(abs(normalizedVariance) - float4(range, range, range, range), 0 , 1);
	// normalizedVariance = normalizedVariance * normalizedVariance * ( 3 - 2 * normalizedVariance);
	history.weightCurve.setForce(normalizedVariance.w);
	history.weightCurve.setScale(normalizedVariance.w);
	history.weightCurve.update();
	
	float3 weightedAvg = history.weightedAverage(history.weightCurve).xyz;
	float3 adaptive = adaptiveAverage(surfel.indirectLighting, weightedAvg, normalizedVariance.w);
	surfel.indirectLighting = clamp(adaptive,
																weightedAvg - sqrt(variance.xyz), weightedAvg + sqrt(variance.xyz));
	// surfel.indirectLighting = history.average().xyz;

	surfel.age++;
}


[RootSignature(SurfelGI_RootSig)]
[numthreads(16, 16, 1)]

void main( uint3 threadId : SV_DispatchThreadID, uint3 localThreadId : SV_GroupThreadID, uint3 groupId: SV_GroupId )
{
	seed = threadId.x + threadId.y * 1000 + asuint(gTime);	

	//[loop]
	//for(uint kk = 0; kk < 2; kk++) {
		SurfelBucketInfo info = uSurfelBucket[groupId.x * 16 * 16 * 1 + localThreadId.x * 16 * 1 + localThreadId.y * 1];

		uint i = info.startIndex;
		while(i < info.startIndex + info.currentCount) {
			uSurfels[i] = uSurfelsPreviousFrame[i];
			updateSurfels(uSurfels[i], uSurfelsHistory[i]);
			i++;
		}
	//}
	// for(uint j = 0; j < hash; j++) {
}


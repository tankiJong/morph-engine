#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"

#define DeferredLighting_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 6), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 4), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"



Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<vertex_t> gVerts: register(t14);

RWTexture2D<float4> uTexScene: register(u0);
RWTexture2D<float4> gIndirect: register(u1);
RWTexture2D<float4> gTexAO: register(u2);
RWTexture2D<float4> uSpawnChance: register(u3);




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
	Ray ray;

	float maxDist = length(gLight.position - surfacePosition);
	ray.direction = normalize(gLight.position - surfacePosition);

	ray.position = surfacePosition + surfaceNormal * 0.0001f;

	Contact c = trace(ray);


	if(c.valid && c.t < maxDist) return float3(0,0,0);
	
	return Diffuse(surfacePosition, surfaceNormal, float3(1, 1, 1), gLight);

}

float SpatialGauss(float pixLen, float3 pos1, float3 pos2, float3 norm1, float3 norm2)
{
	float dis1 = distance(pos1, pos2) * distance(pos1, pos2);

	float4 forward = float4(0, 0, -1, 1);
	forward = mul(inverse(view), forward);

	float dis2 = dot(forward.xyz, pos2 - pos1);
	dis2 *= dis2;
	float dist = abs(dis1 - dis2);

	static const float minZ = .1f;

	float omiga	= .3f / pixLen;

	omiga = min(omiga, 2.0f / minZ);
	float beta = omiga * .5f;


	return (1 / sqrt(2 * 3.14) / beta) * exp(-1.f * dist / (2 * beta * beta)); 
}

float RangeGauss(int x, float sigma)
{
	float x_p = x;
	return exp(-1.f * (x_p * x_p) / (2 * sigma * sigma));
}

float Ambient(uint2 maxsize, uint2 pix, float3 position, float3 normal) {
	// return float4(1, 1, 1, 1);
	float weight = 0;

	float hitDist = gTexAO[pix].w;
	int step = ceil(clamp(hitDist * 16.f / 5.f , 3.f, 16.f));
	// int step = 3;
	float ao = 0;
	for(int i = -step; i <= step; i++) {
		for(int j = -step; j <= step; j++) {
			uint2 samplePix = pix + uint2(i, j);
			if(samplePix.x >= maxsize.x || samplePix.y >= maxsize.y) continue;

			float3 samplePosition = gTexPosition[samplePix].xyz;
			float3 sampleNormal = gTexNormal[samplePix].xyz * 2.f - 1.f;

			float dist = distance(samplePosition, position);
			float sigma = .3f * 1.f / dist;

			float pixLen = sqrt(uSpawnChance[samplePix].x);
			float wei = SpatialGauss(pixLen, samplePosition, position, sampleNormal, normal);
			
			ao += wei * gTexAO[samplePix].x;
			weight += wei;
		}
	}

	return ao;
}

float3 PhongLighting(uint2 pix)
{
	float3 surfacePosition = gTexPosition[pix].xyz;
	float3 surfaceNormal = gTexNormal[pix].xyz * 2.f - 1.f;
	float3 surfaceColor = gTexAlbedo[pix].xyz;
	float3 eyePosition;
	{
		float4 eye = mul(inverse(view), mul(inverse(projection), float4(0,0,0,1.f)));
		eyePosition = eye.xyz / eye.w;
	}

	uint2 aoSize;

	gTexAO.GetDimensions(aoSize.x, aoSize.y);

  float ambient = Ambient(aoSize, pix, surfacePosition, surfaceNormal);
	
	return float3(ambient, ambient, ambient);
	const float SPECULAR_AMOUNT = 1.f;
	const float SPECULAR_POWER = 2.f;
  // float3 diffuse = float3(0,0,0);
  float3 diffuse = computeDiffuse(surfacePosition, surfaceNormal);
  float3 specular = Specular(surfacePosition, surfaceNormal, 
														 normalize(eyePosition - surfacePosition), SPECULAR_AMOUNT, SPECULAR_POWER, gLight);

	float3 indirect = gIndirect[pix / 2].xyz / gIndirect[pix / 2].w * ambient;
	
	// return indirect;
	// only A ray will enter the pixel, so the color need to divide by 2PI
  float3 color = (diffuse / 3.14159f + 2 * indirect) * surfaceColor / ( 2 * 3.141592f )/* + specular*/;

	const float GAMMA = 1.f / 2.1;
	color =  pow(color, float3(GAMMA, GAMMA, GAMMA)) ;

  return clamp(color, float3(0.f, 0.f, 0.f), float3(1.f, 1.f, 1.f));
}

[RootSignature(DeferredLighting_RootSig)]
[numthreads(8, 8, 1)]
void main( uint3 threadId: SV_DispatchThreadID )
{
	uint2 pix = threadId.xy;

	uint2 size;
	uTexScene.GetDimensions(size.x, size.y);
	
	if(pix.x > size.x || pix.y > size.y) return;

	float3 color = PhongLighting(pix);
	uTexScene[pix] = float4(color, 1.f);
}


#include "Common.hlsli"
#include "Surfel.hlsli"
#include "Lighting.hlsli"

#define DeferredLighting_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
     RootSig_Common \
		"DescriptorTable(SRV(t10, numDescriptors = 6), visibility = SHADER_VISIBILITY_ALL)," \
		"DescriptorTable(UAV(u0, numDescriptors = 3), SRV(t20, numDescriptors = 1), visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_ALL),"



Texture2D<float4> gTexAlbedo:   register(t10);
Texture2D<float4> gTexNormal:   register(t11);
Texture2D<float4> gTexPosition: register(t12);
Texture2D gTexDepth: register(t13);
StructuredBuffer<Prim> gVerts:	register(t14);
StructuredBuffer<BVHNode> gBvh: register(t15);

RWTexture2D<float4> uTexScene: register(u0);
RWTexture2D<float4> gIndirect: register(u1);
RWTexture2D<float4> uSpawnChance: register(u2);
Texture2D<float4> gTexAO: register(t20);

float3 computeDiffuse(float3 surfacePosition, float3 surfaceNormal) {
	// return float3(0,0,0);
	Ray ray;

	float maxDist = length(gLight.position - surfacePosition);
	ray.direction = normalize(gLight.position - surfacePosition);

	ray.position = surfacePosition + surfaceNormal * 0.0001f;

	Contact c = trace(ray, gBvh, gVerts);


	if(c.valid && c.t < maxDist) return float3(0,0,0);
	
	// Randomf r = rnd01(seed);
	// seed = r.seed; 
	float x = .37568f;
						
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

float SpatialGauss(float3 pos1, float3 pos2, float3 norm1, float3 norm2, float beta)
{
	float dis1 = distance(pos1, pos2) * saturate(dot(norm1, norm2));
	
	float4 forward = float4(0, 0, 1, 1);
	forward = mul(inverse(view), forward);
	forward.xyz /= forward.w;

	float3 norm = normalize(forward.xyz);

	float3 d = pos2 - pos1;

	float proj = dot(d, norm);
	// d = d + proj * (-norm); 

	float dist = length(d) * length(d);


	return (1 / (abs(proj) + 1)) * saturate(dot(norm1, norm2)) * (1 / (2 * 3.14 * beta * beta)) * exp(-1.f * dist / (2 * beta * beta)); 
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
	float step = saturate(hitDist / 1.f);

	step = clamp(8 * ( step * step ),1.f, 8.f);
	//int step = 16;
	float ao = 0;
	float2 pixLen = uSpawnChance[pix].xy;

	static const float minZ = .1f;

	float omiga	= .3f / pixLen.x ;
	omiga = min(omiga, 2.0f / minZ) / float(step);

	float beta = .5f / omiga;
	// beta = step * beta / 3.f;
	// int step = ceil(beta * 1.5 / pixLen);
	for(int i = -16; i <= 16; i++) {
		if(int(pix.x) + i >= int(maxsize.x) || int(pix.x) + i < 0) continue;

		for(int j = -16; j <= 16; j++) {
			int2 samplePix = pix + int2(i, j);
			if(samplePix.y >= int(maxsize.y) || int(samplePix.y) + j < 0) continue;

			float3 samplePosition = gTexPosition[samplePix].xyz;
			float3 sampleNormal = gTexNormal[samplePix].xyz * 2.f - 1.f;

			if(!any(sampleNormal)) continue; 

			float4 ssample = gTexAO[samplePix].x;
			float wei = SpatialGauss(samplePosition, position, sampleNormal, normal, beta);
								//* RangeGauss(abs(i) + abs(j), float(step) * 2.f / 3.f);
			
			ao += wei * wei * ssample.x;
			weight += wei * wei;
			//uTexScene[samplePix] = float4(wei, wei, wei, 1.f);

		}
	}

	return ao / weight;
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

	if(length(surfaceNormal) < 0.5f) return float3(0,0,0); 

	uint2 aoSize;

	gTexAO.GetDimensions(aoSize.x, aoSize.y);

  // float ambient = Ambient(aoSize, pix, surfacePosition, surfaceNormal);
	// ambient = clamp(smoothstep(0, 1, ambient), 0, 1);
  float3 ambient = gTexAO[pix].xxx / surfaceColor;
  // float ambient = 1;
	// gTexAO[pix] = float4(ambient, ambient, ambient, 1.f);
	// return float3(ambient, ambient, ambient);

  // float3 diffuse = float3(0,0,0);
  // float3 diffuse =  Diffuse(surfacePosition, surfaceNormal, gLight.color.xyz, gLight);
  float3 diffuse = computeDiffuse(surfacePosition, surfaceNormal);
  // float3 specular = Specular(surfacePosition, surfaceNormal, 
	// 													 normalize(eyePosition - surfacePosition), SPECULAR_AMOUNT, SPECULAR_POWER, gLight);

	float3 indirect = ( gIndirect[pix / 2].xyz / gIndirect[pix / 2].w ) * (2 * M_PI) * ambient ;
	
	// return indirect;
  float3 color = ( diffuse + indirect ) * surfaceColor / M_PI /* + specular*/;

	const float GAMMA = 1.f / 2.1;
	color =  pow(color, float3(GAMMA, GAMMA, GAMMA));
	color = saturate(color);
  return color;
}

[RootSignature(DeferredLighting_RootSig)]
[numthreads(8, 8, 1)]
void main( uint3 threadId: SV_DispatchThreadID )
{
	// uint seed = threadId.x + threadId.y * 1000 + asuint(gTime);	


	uint2 pix = threadId.xy;

	uint2 size;
	uTexScene.GetDimensions(size.x, size.y);
	
	if(pix.x > size.x || pix.y > size.y) return;

	float3 color = PhongLighting(pix);
	// float v = rnd01(seed).value;
	// float v = rnd01(seed).value	+ gFrameCount * uTexScene[pix];
	// v = v / (1 + gFrameCount);
	// uTexScene[pix] = float4(v, v, v, 1.f);
	uTexScene[pix] = float4(color, 1.f);
}





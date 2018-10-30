#include "../Shader/ShaderInterop.h"
#include "./Common.hlsli"
#include "../Shader/Math.hlsl"

#define TOTAL_HISTORY 80u

struct BezierCurve {
  float2 start;
  float2 end;

  float2 tangentStart;
  float2 tangentEnd;

	float smooth;
	float force;
	float scale;

	inline float evaluate(float t) {
		float2 a = lerp(start, tangentStart, t);
		float2 b = lerp(tangentStart, tangentEnd, t);
		float2 c = lerp(tangentEnd, end, t);

		float2 d = lerp(a, b, t);
		float2 e = lerp(b, c, t);

		float2 re = lerp(d, e, t);
		
		return pow(re.y, scale);
	}

	inline void update() {
		tangentStart.x = 1.5f * force;
		tangentStart.y = 0;

		tangentEnd.x = 1.f - force;
		tangentEnd.y = 1.f;

		tangentStart = rotate2D(tangentStart, start, 45 * smooth);
		tangentEnd = rotate2D(tangentEnd, end, 45 * smooth);
	}

	inline float setSmooth(float f) {
		smooth = clamp(f,0,1) * .5f + smooth * .5f;
		return smooth;
	}
	
	inline float setScale(float f) {
		float val = 3.f * clamp(abs(f), 0, 1);
		// scale = val;
		scale = val * .8f + scale * .2f;
		return scale;
	}

	inline float setForce(float f) {
		force = abs(f);
		return force;
		force = force * .2f + clamp(abs(f), 0, 1) * .8f;
		return force;
	}

};

void InitBezierCurve(inout BezierCurve curve) {
	curve.start = float2(0, 0);
	curve.end = float2(1, 1);
	curve.tangentStart = float2(0, 1);
	curve.tangentEnd = float2(0, 1);

	curve.smooth = 0;
	curve.force = 0;
	curve.scale = 0;
}


struct HistoryBuffer {
	// x,y,z are data, w is variance buffer;
  float4 buffer[TOTAL_HISTORY];
  uint nextToWrite;

	inline void write(float3 data, float prevVariance) {
		uint index = nextToWrite % TOTAL_HISTORY;
		float4 temp[TOTAL_HISTORY] = buffer;
		temp[index]	= float4(data, prevVariance);
		buffer = temp;
		nextToWrite++;
	}

	inline uint bufferIndex(uint index) {
		if(nextToWrite < TOTAL_HISTORY) return index;
		return (nextToWrite + index) % TOTAL_HISTORY;
	}
  inline uint size() {
    return clamp(nextToWrite, 0u, TOTAL_HISTORY);
  }

  inline float4 weightedAverage(in BezierCurve weightCurve) {
		float4 sum = 0;
		float weight = 0;
		uint s = size();
		if(s == 0) return 0;
		for(uint i = 0; i < s; i++) {
			float wei = weightCurve.evaluate(float(i) / float(TOTAL_HISTORY) + 0.001f);
			// float wei = float(i) / float(TOTAL_HISTORY) + 0.001f;
			// float wei = 1;
			sum += buffer[bufferIndex(i)] * wei;
			weight += wei;
		}

		return sum / weight;
  }

	inline float4 average() {
		float4 sum = 0;
		uint s = size();
		
		if(s == 0) return float4(1, 1, 1, 1.f);
		for(uint i = 0; i < s; i++) {
			sum += buffer[bufferIndex(i)];
		}

		return sum / float(s);
	}

	// first 3 component is variance of HSL, 4th component is the variance of variance
  inline float4 variance() {
		uint s = 0;
		s = size();
		if(s == 0) return float4(0,0, 0, 0);
		float4 avg = average();

		float4 sum = float4(0, 0, 0, 0);
		for(uint i = 0; i < s; i++) {
			float4 dist = buffer[i] - avg;
			dist *= dist;
			sum += dist;
		}

		return sum / float(s);
  }


	// return [the biggest color][biggest variance], measurement use eular distance
	inline float4 dataMin() {
		float4 mmin = float4(1e9, 1e9, 1e9, 1e9);


		for(uint i = 0; i < size(); i++) {
			float3 hsl = float3(RGBtoHSL(normalize(buffer[i].xyz)).xy, ExtractLfromRGB(buffer[i].xyz));
			mmin = min(mmin, float4(hsl, buffer[i].w));
		}

		return mmin;
	}

	inline float4 dataMax() {
		float4 mmax = float4(-1e-9, -1e-9, -1e-9, -1e-9);


		for(uint i = 0; i < size(); i++) {
			float3 hsl = float3(RGBtoHSL(normalize(buffer[i].xyz)).xy, ExtractLfromRGB(buffer[i].xyz));
			mmax = max(mmax, float4(hsl, buffer[i].w));
		}

		return mmax;
	}


	inline float4 range() {
		if(size() == 0) return float4(1.f, 1.f, 1.f, 1.f);
		float4 dmin = dataMin();
		float4 dmax = dataMax();
		
		return abs(dmax - dmin);
		
	}

	float4 normalizedHSLVariance() {
		if(size() == 0) return float4(1, 1, 1, 1);
		float4 avgData = average();
		float4 avg = float4(float3(RGBtoHSL(normalize(avgData.xyz)).xy, ExtractLfromRGB(avgData.xyz)), avgData.w);

		float4 normalizedHSLAverage = float4(0,0,0,0);
		
		float4 rng = range();
		float4 mmin = dataMin();
		for(uint i = 0; i < size(); i++) {
			float4 len = float4(float3(RGBtoHSL(normalize(buffer[i].xyz)).xy, ExtractLfromRGB(buffer[i].xyz)), buffer[i].w);
			len = (len - mmin) / rng;
			normalizedHSLAverage += len;
		}
		normalizedHSLAverage /= size();

		float4 vari = float4(0,0,0,0);

		for(uint i = 0; i < size(); i++) {
			float4 len = float4(float3(RGBtoHSL(normalize(buffer[i].xyz)).xy, ExtractLfromRGB(buffer[i].xyz)), buffer[i].w);
			len = (len - mmin) / rng;
			len = len - normalizedHSLAverage;
			vari +=  len * len;
		}
		vari /= size();

		return vari;
	}
};

struct surfel_t {
  float3 position;

  float3 normal;

  float3 color;

  float3 indirectLighting;
  float age;

  float id;

	BezierCurve weightCurve;
  HistoryBuffer history;
	float2 __padding;
};


static const float SURFEL_RADIUS = 0.035f;
static const uint TILE_SIZE = 16;


float isCovered(float3 position, float3 normal, surfel_t surfel) {

	// 1. their normal agree with each other
	float dp = dot(normal, surfel.normal);

	if(dp < 0) return 0;

	// 2. if this is too far, no
	float dist = distance(position, surfel.position);

	float dirDot = dot((position - surfel.position), surfel.normal);

	// too far and not in the same plane
	if(dist > SURFEL_RADIUS) return 0;

	// 2. project the point to the surfel plane, they are close enough
	float3 projected = position - dirDot * surfel.normal;
	
	float d = distance(projected, surfel.position);
	float dd = 1 / (d*d/(SURFEL_RADIUS*SURFEL_RADIUS) + 1) * dp;
	return dd;			// [1, 0)

}

float2 GetProjectedDistanceFromSurfel(float3 position, in float3 sposition, in float3 snormal) {
	float dirDot = dot((position - sposition), snormal);
	
	float3 projected = position - dirDot * snormal;
	
	float d = distance(projected, sposition);

	return float2(d, abs(dirDot));
}

static const uint BUCKET_COUNT = 0xf;		 // 0x?zyx
static const float BUCKET_SIZE = 3 * SURFEL_RADIUS;

struct SurfelBucketInfo {
	uint startIndex;
	uint endIndex;
	uint currentCount;
};

uint SpatialHash(float3 position) {
	uint3 hash = uint3(position / BUCKET_SIZE);

	return (0x000f & hash.x) | 
				 (0x00f0 & (hash.y << 4)) |
				 (0x0f00 & (hash.z << 8));
}

uint3 GetSpatialHashComponent(uint hash) {
	uint3 component;
	component.x = 0x000f & hash;
	hash >>= 4;
	component.y = 0x000f & hash;
	hash >>= 4;
	component.z = 0x000f & hash;

	return component;
}

uint GetSpatialHashFromComponent(uint3 components) {
	return (0x000f & components.x) | 
				 (0x00f0 & (components.y << 4)) |
				 (0x0f00 & (components.z << 8));
}

/* { ([0x0000][0x0001]....)([0x0010][0x0011]....)------([0x00f0][0x00f1]....) }
 	 { ([0x0100][0x0101]....)([0x0110][0x0111]....)------([0x01f0][0x01f1]....) }
	 { ([0x0200][0x0201]....)([0x0210][0x0211]....)------([0x02f0][0x02f1]....) }
	 .....
	 { ([0x0f00][0x0f01]....)([0x0f10][0x0f11]....)------([0x0ff0][0x0ff1]....) }

	 actual element in the bucket: [startIndex, endIndex)
 */
void SpatialToIndexRange(uint hash, uint arraySize, out uint startIndex, out uint endIndex) {
	uint bucketCount = BUCKET_COUNT * BUCKET_COUNT * BUCKET_COUNT;
	uint offset = ceil(float(arraySize) / float(bucketCount));

	uint3 component;
	component = GetSpatialHashComponent(hash);

	startIndex = offset * (component.z * BUCKET_COUNT * BUCKET_COUNT + component.y * BUCKET_COUNT + component.x);
	endIndex = startIndex + offset;
}

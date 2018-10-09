#define TOTAL_HISTORY 32
struct surfel_t {
  float3 position;
  float __padding0;

  float3 normal;
  float __padding1;
  
  float3 color;
  
  float3 indirectLighting;
  float age;
  
  uint nextToWrite;
  float id;
  float2 __padding3;
  
  float4 history[32];
};

static const float SURFEL_RADIUS = 0.02f;
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
	float dd = ( SURFEL_RADIUS - d )*( SURFEL_RADIUS - d );
	return dd;			// [1, 0)

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


struct surfel_t {
  float3 position;
  float3 normal;
	float3 color;
	float3 indirectLighting;
	float age;
	float id;
};

static const float SURFEL_RADIUS = 1.f;
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
	float dd = ( 1 - d )*( 1 - d );
	return dd;			// [1, 0)

}
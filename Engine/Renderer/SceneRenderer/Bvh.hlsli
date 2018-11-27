

struct aabb3 {
	float3 mins;
	float3 maxs;
	
	/*
	*   7 ----- 6
	*  / |    / |
	* 4 -+-- 5  |
	* |  3 --|- 2
	* | /    | /
	* 0 ---- 1
	*
	*/
	inline void corners(inout float3 v[8]) {
		v[0] =float3( mins.x, mins.y, mins.z ); // min
		v[1] =float3( maxs.x, mins.y, mins.z );
		v[2] =float3( maxs.x, mins.y, maxs.z );
		v[3] =float3( mins.x, mins.y, maxs.z );

		v[4] =float3( mins.x, maxs.y, mins.z );
		v[5] =float3( maxs.x, maxs.y, mins.z );
		v[6] =float3( maxs.x, maxs.y, maxs.z ); // max
		v[7] =float3( mins.x, maxs.y, maxs.z );
	};
};


struct BVHNode {
  aabb3 bounds: BOUND_BOX;
  struct {
    uint start; // inclusive
    uint end;   // exclusive
  } childRange: CHILD_NODE_OFFSET, 
		triRange: PRIM_RANGE;
};


struct Prim {
	float3 p[3];
	float4 c[3];
	float2 uv[3];
	aabb3 bounds;
};


struct Ray {
	float3 position;
	float3 direction;
	float3 directionInv;
};

struct Contact {
	float4 position;
	float3 normal;
	float t;
	bool valid;
};

static uint boxIntersectCalledTimes = 0;


#define inbox(v, b1, b2) ((v.x>=b1.x) && (v.y>=b1.y) && (v.z>=b1.z) && (v.x<=b2.x) && (v.y<=b2.y) && (v.z<=b2.z))

bool intersect(in Ray ray, in aabb3 box)
{
	boxIntersectCalledTimes++;
  const float3 t0 = (box.mins - ray.position) / ray.direction;
  const float3 t1 = (box.maxs - ray.position) / ray.direction;
 
  const float3 tmax = max(t0, t1);
  const float3 tmin = min(t0, t1);
 
  const float a1 = min(tmax.x, min(tmax.y, tmax.z));
  const float a0 = max( max(tmin.x,tmin.y), tmin.z);
 
  return (a1 >= a0 && a1 >= 0 && !isnan(box.mins.x));
}


Contact triIntersection(float3 a, float3 b, float3 c, float color, Ray ray) {

	Contact contact;

	float3 ab = b - a;
	float3 ac = c - a;
	float3 normal = normalize(cross(ac, ab));
	contact.normal = normal;

	contact.valid = dot(normal, ray.direction) < 0;

	float t = (dot(a, normal) - dot(ray.position, normal)) / dot(ray.direction, normal);
	contact.t = t;
	contact.position.xyz = ray.position + t * ray.direction;
	contact.position.w = color;

	float3 p = contact.position.xyz;
	contact.valid = contact.valid && dot(cross(p - a, b - a), normal) >= 0;
	contact.valid = contact.valid && dot(cross(p - b, c - b), normal) >= 0;
	contact.valid = contact.valid && dot(cross(p - c, a - c), normal) >= 0;

	return contact;
}

Contact trace(Ray ray, 
							in StructuredBuffer<BVHNode> nodes, 
							in StructuredBuffer<Prim> prims, out float4 color) {
	
	const uint MAX_COLLISION_RECORD = 6;
	uint currentNodeIndex = 1;
	uint nextRecord = 0;

	BVHNode collisionNodes[MAX_COLLISION_RECORD];

	uint numNode, _;

	nodes.GetDimensions(numNode, _);

	uint lastNodeIndex = currentNodeIndex;
	
	uint recordStep = 0;

	while(nextRecord < MAX_COLLISION_RECORD && currentNodeIndex < numNode) {
		BVHNode node = nodes[currentNodeIndex];

		bool intersected = intersect(ray, node.bounds);

		if(intersected) {
			if(node.childRange.end == node.childRange.start) {
				collisionNodes[nextRecord] = node;
				nextRecord++;
			}

			currentNodeIndex++;

		}	else {
			currentNodeIndex = max(node.childRange.end, currentNodeIndex + 1);
		}

		// if(currentNodeIndex == lastNodeIndex) {
		// 	break;
		// }
		// 
		// lastNodeIndex = currentNodeIndex;
	};
	
	Contact contact;
	contact.t = 1e6;
	contact.valid = false;

	for(uint i = 0; i < nextRecord; i++) {

		for(uint k = collisionNodes[i].triRange.start; k < collisionNodes[i].triRange.end; k++) {
			Prim prim = prims[k];
			Contact c = triIntersection(prim.p[0], prim.p[1], prim.p[2], 1, ray);
			bool valid = c.valid && (c.t < contact.t) && (c.t > 0.f);	 // equal to zero avoid the fail intersaction in the corner	edge
			if(valid)	{
				contact = c;
				color = prim.c[0];
			}
		}
	}
	
	return contact;
}

Contact trace(Ray ray, 
							in StructuredBuffer<BVHNode> nodes, 
							in StructuredBuffer<Prim> prims) {
	float4 color;
	return trace(ray, nodes, prims, color);
}

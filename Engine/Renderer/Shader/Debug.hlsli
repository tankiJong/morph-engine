

#define DebugDraw_RootSig "DescriptorTable(UAV(u100, numDescriptors = 2), visibility = SHADER_VISIBILITY_ALL),"

struct v_debug {
	float3 position;
	float4 color;
	float2 uv;
};

RWStructuredBuffer<v_debug> uDebugMesh: register(u100);
RWStructuredBuffer<uint> uDebugVertCount: register(u101);

static bool enableDebug;	

void DebugDrawLine(float3 fromPos, float3 toPos, float4 fromColor, float4 toColor, float2 fromUV, float2 toUV) {
	if(!enableDebug) return;
	uint nextToWrite;
	InterlockedAdd(uDebugVertCount[0], 2, nextToWrite);

	v_debug from;
	v_debug to;

	from.position = fromPos;
	from.color = fromColor;
	from.uv = fromUV;

	to.position = toPos;
	to.color = toColor;
	to.uv = toUV;

	uDebugMesh[nextToWrite] = from;
	uDebugMesh[nextToWrite + 1] = to;
}


void DebugDrawLine(float3 fromPos, float3 toPos) {
	DebugDrawLine(fromPos, toPos, 1.f.xxxx, 1.f.xxxx, 0.f.xx, 1.f.xx);
}


void DebugDrawCube(float3 mins, float3 maxs, float4 color) {
	float3 verts[8] = {
		float3(mins.x, mins.y, mins.z),
		float3(maxs.x, mins.y, mins.z),
		float3(maxs.x, maxs.y, mins.z),
		float3(mins.x, maxs.y, mins.z),
		float3(mins.x, mins.y, maxs.z),
		float3(maxs.x, mins.y, maxs.z),
		float3(maxs.x, maxs.y, maxs.z),
		float3(mins.x, maxs.y, maxs.z)
	};
	
	DebugDrawLine(verts[0], verts[1], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[1], verts[2], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[2], verts[3], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[3], verts[0], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[4], verts[5], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[5], verts[6], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[6], verts[7], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[7], verts[4], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[0], verts[4], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[1], verts[5], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[2], verts[6], color, color, 0.f.xx, 0.f.xx);
	DebugDrawLine(verts[3], verts[7], color, color, 0.f.xx, 0.f.xx);
}

void DebugDrawPoint(float3 position, float4 color) {
	DebugDrawCube(position - .03f.xxx, position + .03f.xxx, color);
}
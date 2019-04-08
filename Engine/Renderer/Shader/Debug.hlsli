

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
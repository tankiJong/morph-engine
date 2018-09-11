#include "./Common.hlsli"

//	https://docs.microsoft.com/en-us/windows/desktop/direct3d12/dynamic-indexing-using-hlsl-5-1

StructuredBuffer<float3> cPosition: register(t0);
StructuredBuffer<uint> cColor: register(t1);

AppendStructuredBuffer<vertex_t> cVerts: register(u0);

float4 asColor(uint c) {
	uint r,g,b,a;

	r = c & 0xff;
	c >>= 8;

	g = c & 0xff;
	c >>= 8;
	
	b = c & 0xff;
	c >>= 8;
	
	a = c & 0xff;

	return float4(r, g, b, a) / float4(255.f, 255.f, 255.f, 255.f);
}


[numthreads(1, 1, 1)]
void main( uint3 threadId : SV_DispatchThreadID, uint3 groupID: SV_GroupThreadID )
{
	uint count, _;
	cPosition.GetDimensions(count, _);

	for(uint i = 0; i < count; i++) {
		vertex_t v;
		
		v.position = float4(cPosition[i], i);
		v.color = asColor(cColor[i]);
		
		
		cVerts.Append(v);
	}
}
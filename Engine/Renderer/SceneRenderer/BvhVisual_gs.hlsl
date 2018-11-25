#include "BvhVisual.hlsli"


/* d - c
	 | / |
   a - b
*/

void quad(float3 a, float3 b, float3 c, float3 d, inout TriangleStream<PSInput> stream) {
	PSInput x,y,z,w;
	x.position = float4(a, 1);
	y.position = float4(b, 1);
	z.position = float4(c, 1);
	w.position = float4(d, 1);


	stream.Append(x);
	stream.Append(y);
	stream.Append(z);
	stream.RestartStrip();

	stream.Append(x);
	stream.Append(z);
	stream.Append(w);
	stream.RestartStrip();
}

[RootSignature(BvhVisual_RootSig)]
[maxvertexcount(36)]
void main(point GSInput node[1], inout TriangleStream<PSInput> stream) {

	float3 vertices[8];
	node[0].node.bounds.corners(vertices);
	
	if(node[0].node.childRange.end != node[0].node.childRange.start) return;

	[unroll(8)]
	for(uint i = 0; i < 8; i++) {
		float4 v = mul(projection, mul(view, float4(vertices[i], 1.f)));
		vertices[i] = v.xyz / v.w;
	}

	quad(vertices[3], vertices[2], vertices[1], vertices[0], stream);
  quad(vertices[4], vertices[5], vertices[6], vertices[7], stream);
  quad(vertices[0], vertices[1], vertices[5], vertices[4], stream);
  quad(vertices[1], vertices[2], vertices[6], vertices[5], stream);
  quad(vertices[2], vertices[3], vertices[7], vertices[6], stream);
  quad(vertices[3], vertices[0], vertices[4], vertices[7], stream);
}
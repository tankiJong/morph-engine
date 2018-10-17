#include "GenGBuffer.hlsli"

[RootSignature(GenBuffer_RootSig)]
PSInput main(
	float3 position: POSITION,
	float4 color:    COLOR,
	float2 uv:       UV,
	float3 normal:   NORMAL,
	float3 tangent:  TANGENT) {

  PSInput result;

	result.worldPosition = position;
  result.position = mul(mul(projection, view), float4(position, 1.f));
  result.color = color;
  result.uv = uv;
  result.normal = normal;
	result.tangent = tangent;
	result.eyePosition = inverse(view)._14_24_34;
	float4 prevViewPosition = mul(prev_view, float4(position, 1.f));

	float4 viewPosition = mul(view, float4(position, 1.f));
	result.deltaViewOffset = viewPosition / viewPosition.w - prevViewPosition / prevViewPosition.w;
  return result;
}

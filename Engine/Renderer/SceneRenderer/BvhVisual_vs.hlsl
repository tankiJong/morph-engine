#include "BvhVisual.hlsli"



[RootSignature(BvhVisual_RootSig)]
GSInput main(uint vertexId: SV_VertexID) {
	BVHNode node = gBvh[vertexId];
	GSInput input;

	input.node = node;
	input.index = vertexId;
	return input;
}


struct PSInput {
  float4 position : SV_POSITION;
  float4 color : COLOR;
};

struct PSOutput {
	float4 color: SV_TARGET0;
};

cbuffer cCamera : register(b1) {
  float4x4 projection;
  float4x4 view;
	float4x4 prev_projection;
  float4x4 prev_view;
};

cbuffer cModel: register(b2) {
	float4x4 model;
}

Texture2D<float4> gFont: register(t1);

SamplerState gSamepler;

PSInput vmain(
	float3 position: POSITION,
	float4 color:    COLOR) {
	PSInput input;

	input.position = mul(model, mul(projection, mul(view, float4(position, 1.f))));
	input.color = color;

	return input;
}

float RangeMap( float x, float d0, float d1, float r0, float r1 )
{
   return (x - d0) / (d1 - d0) * (r1 - r0) + r0;    
}


PSOutput pmain(PSInput input) {
	PSOutput output;
	
  output.color = input.color; 

	 return output;
}
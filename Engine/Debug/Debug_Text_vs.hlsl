
struct PSInput {
  float4 position : SV_POSITION;
  float4 color : COLOR;
  float2 uv: UV;
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

Texture2D<float4> gFont: register(t1);

SamplerState gSamepler;

PSInput vmain(
	float3 position: POSITION,
	float4 color:    COLOR,
	float2 uv:       UV) {
	PSInput input;

	input.position = mul(projection, mul(view, float4(position, 1.f)));
	input.color = color;
	input.uv = float2(uv.x, 1 - uv.y);

	return input;
}

float RangeMap( float x, float d0, float d1, float r0, float r1 )
{
   return (x - d0) / (d1 - d0) * (r1 - r0) + r0;    
}


PSOutput pmain(PSInput input) {
	PSOutput output;
	float alpha = gFont.Sample(gSamepler, input.uv).x;
   
   if (alpha < .45f) {
      discard; 
   }

   alpha = RangeMap( alpha, .45f, .5f, 0.0f, 1.0f );
   alpha = smoothstep(0.f, 1.f, alpha);
   output.color = float4( input.color.xyz, input.color.w * alpha ); 

	 return output;
}
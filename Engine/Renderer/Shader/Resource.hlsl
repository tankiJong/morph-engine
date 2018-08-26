#ifndef __RESOURCE_INCLUDED__
#define __RESOURCE_INCLUDED__		


struct light_info_t {
  float4 color;

  float3 attenuation;
  float dotInnerAngle;

  float3 specAttenuation;
  float dotOuterAngle;

  float3 position;
  float directionFactor;

  float3 direction;
  float __pad00;

  float4x4 vp;
};

cbuffer cTime: register(b0) {
	float time;
}

cbuffer cCamera : register(b1) {
  float4x4 projection;
  float4x4 view;
};

cbuffer cModel: register(b2) {
	float4x4 model;
}

cbuffer cLight: register(b3) {
  light_info_t light;
}

SamplerState gSampler : register(s0);

Texture2D gTexAmbient:  register(t0);
Texture2D gTexAlbedo:   register(t1);
Texture2D gTexNormal:   register(t2);
Texture2D gTexSpecular: register(t3);

#define TEXTURE_USER_1 t4
#define TEXTURE_USER_2 t5


#endif
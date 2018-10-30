
#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 12
#include "fxaa.hlsli"


#include "fullscreen.hlsli"

struct PSOutput {
	float4 color;
};


PSOutput main(PostProcessingVSOutput input) {

}
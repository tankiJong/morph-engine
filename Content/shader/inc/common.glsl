#ifndef DEFINE_COMMON_F
#define DEFINE_COMMON_F

#define UNIFORM_TIME    1
#define UNIFORM_CAMERA  2
#define UNIFORM_LIGHT   3

layout(std140, binding = UNIFORM_CAMERA) uniform cameraBlock {
	mat4 PROJECTION;
	mat4 VIEW;
};

#endif
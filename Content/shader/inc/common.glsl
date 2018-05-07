#ifndef DEFINE_COMMON_F
#define DEFINE_COMMON_F

#define UNIFORM_TIME    1
#define UNIFORM_CAMERA  2
#define UNIFORM_TRANSFORM  3
#define UNIFORM_LIGHT   4
#define UNIFORM_USER_1   12

layout(std140, binding = UNIFORM_CAMERA) uniform cameraBlock {
	mat4 PROJECTION;
	mat4 VIEW;
};

layout(std140, binding = UNIFORM_TRANSFORM) uniform transoformBlock {
	mat4 MODEL;
};
#endif
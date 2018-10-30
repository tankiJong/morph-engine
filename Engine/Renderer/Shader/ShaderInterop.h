#ifndef _WICKEDENGINE_SHADERINTEROP_H_
#define _WICKEDENGINE_SHADERINTEROP_H_

#ifdef __cplusplus // not invoking shader compiler, but included in engine source

// Application-side types:

class ivec3;
class ivec2;
class vec3;
class uvec3;
struct vec4;
class uvec2;
class vec2;
class mat44;
typedef mat44 float4x4;
typedef vec2 float2;
typedef vec3 float3;
typedef vec4 float4;
// typedef uint uint;
typedef uvec2 uint2;
typedef uvec3 uint3;
// typedef uvec4 uint4;

typedef ivec2 int2;
typedef ivec3 int3;
// typedef ivec4 int4;



#else


#endif





#endif


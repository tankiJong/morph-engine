#version 420 core

#include "../inc/common.glsl"
// Uniforms ==============================================

// Attributes ============================================
// Inputs
in vec3 POSITION;
in vec4 COLOR;
in vec2 UV; 

#ifdef _LIT
in vec3 NORMAL;   // new
in vec4 TANGENT;
#endif

// Outputs
out vec2 passUV; 
out vec4 passColor; 

#ifdef _LIT
out vec3 passWorldPos;     // new
out vec3 passWorldNormal;  // new
out vec3 passEyeDir;
out vec3 passTangent;
out vec3 passBiTangent;
#endif

// Entry Point ===========================================
void main()
{
   vec4 local_pos = vec4( POSITION, 1.0f );  

   vec4 world_pos = MODEL * local_pos;
   vec4 clip_pos = PROJECTION * VIEW * MODEL * local_pos; 

   passUV = UV;
   passColor = COLOR; 

#ifdef _LIT
   // new
   passWorldPos = world_pos.xyz;  
   passWorldNormal = (vec4( NORMAL, 0.0f ) * MODEL).xyz; 
   passTangent = TANGENT.xyz;
   passBiTangent = cross(TANGENT.xyz, NORMAL) * TANGENT.w;
   vec3 eyePos = inverse(VIEW)[3].xyz;
   passEyeDir = eyePos - passWorldPos;
#endif

   gl_Position = clip_pos; // we pass out a clip coordinate
}
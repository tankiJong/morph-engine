#version 420 core
in vec3 POSITION;
in vec4 COLOR;       // NEW - GLSL will use a Vector4 for this; 
in vec2 UV;  

#include "../inc/common.glsl"


layout(std140, binding = UNIFORM_USER_1) uniform cTintBlock {
	vec4 tint;
};

out vec2 passUV; 
out vec4 passColor;
out vec3 passPos;

void main() 
{
   vec4 local_pos = vec4( POSITION, 1 ); 
   vec4 clip_pos = PROJECTION * VIEW * local_pos; 

   passColor = COLOR * tint; // pass it on. 
   passUV = UV; 
   gl_Position = clip_pos; 
}
#version 420 core

#include "../inc/common.glsl"

in vec3 POSITION;
in vec4 COLOR;       // NEW - GLSL will use a Vector4 for this; 
in vec2 UV;         


out vec2 passUV; 
out vec4 passColor;  // NEW - to use it in the pixel stage, we must pass it.


void main() 
{
	vec4 local_pos = vec4( POSITION, 1.0f );	

	vec4 world_pos = MODEL * local_pos; // assume local is world for now; 
	vec4 camera_pos = VIEW * local_pos; 
	vec4 clip_pos = PROJECTION * camera_pos; 

	passColor = COLOR; 
	passUV = UV;
	gl_Position = clip_pos; // we pass out a clip coordinate
}
#version 420 core

in vec3 POSITION;
in vec4 COLOR;       // NEW - GLSL will use a Vector4 for this; 
in vec2 UV;         

layout(std140, binding = 2) uniform cameraBlock {
   mat4 PROJECTION;
   mat4 VIEW;
};

out vec2 passUV; 
out vec4 passColor;  // NEW - to use it in the pixel stage, we must pass it.
out vec3 passPos;
out vec4 ViewMat;


void main() 
{
   vec4 local_pos = vec4( POSITION, 1 ); 
   vec4 clip_pos = PROJECTION * VIEW * local_pos; 

   passColor = COLOR; // pass it on. 
   passUV = UV; 
   passPos = clip_pos.xyz;
   ViewMat = PROJECTION * VIEW * vec4(0);
   gl_Position = clip_pos; 
}
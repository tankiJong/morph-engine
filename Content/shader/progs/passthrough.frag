#version 420 core

layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec4 passColor; // NEW, passed color
in vec2 passUV; 
in vec3 passPos;
out vec4 outColor; 

void main() 
{
   vec4 diffuse = texture( gTexDiffuse, passUV );
   
   // multiply is component-wise
   // so this gets (diff.x * passColor.x, ..., diff.w * passColor.w)
   outColor = diffuse * passColor;  
}
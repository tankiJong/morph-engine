#version 420 core

in vec4 passColor; // NEW, passed color
in vec2 passUV; 
in vec3 passPos;
out vec4 outColor; 
  
void main() 
{
   outColor = passColor;  
}
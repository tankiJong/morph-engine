#version 420 core

// Suggest always manually setting bindings - again, consitancy with 
// other rendering APIs and well as you can make assumptions in your
// engine without having to query
#include "../inc/common.glsl"
in vec4 passColor; 

out vec4 outColor; 

void main( void )
{
	outColor = passColor; 
}

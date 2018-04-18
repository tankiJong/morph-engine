#version 420 core


// PASS
in vec4 passColor;
in vec2 passUV; 

// OUT
layout(location = 0) out vec4 outColor; 

// TEXTURES
layout(binding = 0) uniform sampler2D gTexDiffuse;

float RangeMap( float x, float d0, float d1, float r0, float r1 )
{
   return (x - d0) / (d1 - d0) * (r1 - r0) + r0;    
}
//--------------------------------------------------------------------------------------
// ENTRY POINT
//--------------------------------------------------------------------------------------
void main( void )
{
   float alpha = texture( gTexDiffuse, passUV ).x;
   
   if (alpha < .5f) {
      discard; 
   }

   outColor = vec4( passColor.xyz, passColor.w); 
}




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
   
   if (alpha < .45f) {
      discard; 
   }

   alpha = RangeMap( alpha, .45f, .5f, 0.0f, 1.0f );
   alpha = smoothstep(0.f, 1.f, alpha);
   outColor = vec4( passColor.xyz, passColor.w * alpha ); 
}




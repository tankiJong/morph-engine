#ifndef DEFINE_MATH_F
#define DEFINE_MATH_F

float rangeMap( float x, float d0, float d1, float r0, float r1 )
{
   return (x - d0) / (d1 - d0) * (r1 - r0) + r0;    
}



#endif
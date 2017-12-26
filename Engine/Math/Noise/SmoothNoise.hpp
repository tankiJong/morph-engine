//-----------------------------------------------------------------------------------------------
// SmoothNoise.hpp
//
#pragma once


/////////////////////////////////////////////////////////////////////////////////////////////////
// Squirrel's Smooth Noise utilities (version 4)
//
// This code is made available under the Creative Commons attribution 3.0 license (CC-BY-3.0 US):
//	Attribution in source code comments (even closed-source/commercial code) is sufficient.
//	License summary and text available at: https://creativecommons.org/licenses/by/3.0/us/
//
// Note: This is work in progress, and has not yet been tested thoroughly.  Use at your own risk.
//	Please report any bugs, issues, or bothersome cases to SquirrelEiserloh at gmail.com.
//
// The following functions are all based on a simple bit-noise function which returns an unsigned
//	integer containing 32 reasonably-well-scrambled bits, based on a given (signed) integer
//	input parameter (position/index) and [optional] seed.  Kind of like looking up a value in an
//	infinitely large [non-existent] table of previously rolled random numbers.
//
// These functions are deterministic and random-access / order-independent (i.e. state-free),
//	so they are particularly well-suited for use in out-of-order (or or-demand) procedural
//	content generation (i.e. that mountain village is the same whether you generated it
//	first or last, ahead of time or just now).
//
// My implementations of fractal and Perlin noise include a few improvements over the stock
//	versions I've seen used:
//	* Functions can take seeds (independent of index/position) with unique-but-consistent results
//	* Each octave is offset (translation/bias) to dramatically reduce multi-octave feedback.
//	* Vector gradients are in power-of-two sets, to avoid modulus ops (uses bitwise masks instead)
//	* Octave persistence and scale are adjustable (not necessarily 0.5 and 2.0)
//	* Multi-octave noise can be "normalized" to be mapped back to within [-1,1], or not
//
// Note: these functions assume the presence of a few simple math utility functions and classes;
//	class Vector2, Vector3, Vector4								// public x,y,z,w floats, basic operators
//	float DotProduct( const Vector2& A, const Vector2& B )		// (Ax*Bx) + (Ay*By)
//	float DotProduct( const Vector3& A, const Vector3& B )		// (Ax*Bx) + (Ay*By) + (Az*Bz)
//	float DotProduct( const Vector4& A, const Vector4& B )		// (Ax*Bx) + (Ay*By) + (Az*Bz) + (Aw*Bw)
//	float SmoothStep3( float inputZeroToOne )					// 3t^2 - 2t^3  : '^' means "power-of"
//
/////////////////////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------------------------
// Smooth/fractal pseudorandom noise functions (random-access / deterministic)
//
// These are less "organic" (and more axial) than Perlin's functions, but simpler and faster.
// 
// <numOctaves>			Number of layers of noise added together
// <octavePersistence>	Amplitude multiplier for each subsequent octave (each octave is quieter)
// <octaveScale>		Frequency multiplier for each subsequent octave (each octave is busier)
// <renormalize>		If true, uses nonlinear (SmoothStep3) renormalization to within [-1,1]
//
float Compute1dFractalNoise( float position, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute2dFractalNoise( float posX, float posY, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute3dFractalNoise( float posX, float posY, float posZ, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute4dFractalNoise( float posX, float posY, float posZ, float posT, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );


//-----------------------------------------------------------------------------------------------
// Perlin noise functions (random-access / deterministic)
//
// Perlin noise is slightly more expensive, but more organic-looking (less axial) than regular
//	square fractal noise, through the use of blended dot products vs. randomized gradient vectors.
//
// <numOctaves>			Number of layers of noise added together
// <octavePersistence>	Amplitude multiplier for each subsequent octave (each octave is quieter)
// <octaveScale>		Frequency multiplier for each subsequent octave (each octave is busier)
// <renormalize>		If true, uses nonlinear (SmoothStep3) renormalization to within [-1,1]
//
float Compute1dPerlinNoise( float position, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute2dPerlinNoise( float posX, float posY, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute3dPerlinNoise( float posX, float posY, float posZ, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );
float Compute4dPerlinNoise( float posX, float posY, float posZ, float posT, float scale=1.f, unsigned int numOctaves=1, float octavePersistence=0.5f, float octaveScale=2.f, bool renormalize=true, unsigned int seed=0 );


//-----------------------------------------------------------------------------------------------
// Simplex noise functions (random-access / deterministic)
//
// Simplex noise (also by Ken Perlin) is theoretically faster than - and supposedly superior to - 
//	Perlin noise, in that it is more organic-looking.  I'm not sure I like the look of it better,
//	however; examples of cross-sectional 4D simplex noise look worse to me than 4D Perlin does.
//
// Also, Simplex noise is based on a regular simplex (2D triangle, 3D tetrahedron, 4-simplex/5-cell)
//	grid, which is slightly more fiddly, so I haven't bothered writing my own yet.
//
// #TODO: Implement simplex noise in 2D, 3D, 4D (1D simplex is identical to 1D Perlin, I think?)
// #TODO: Test actual simplex noise implementation in 2D/3D to compare speeds (branches vs. ops!)
//



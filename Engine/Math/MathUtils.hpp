#pragma once

#define PI (3.1415926535897932384626433832795f)
#include <Math.h>

constexpr float fSQRT_3_OVER_3 = 0.577350269f;
class FloatRange;
class aabb2;
class ivec2;
class IntRange;
class Rgba;
typedef int int32_t;
class vec2;
class Disc2;

float convertRadiansToDegrees (float radians);
float convertDegreesToRadians (float degrees);
float cosDegrees (float degrees);
float sinDegrees (float degrees);
float atan2Degree(float y, float x);
float tanDegree(float degrees);
float getSquaredDistance(const vec2& a, const vec2& b);
float getDistance(const vec2& a, const vec2& b);
float getAngularDisplacement(float startDegrees, float endDegrees);

// it should work for all different scales, no matter degree or radian or whatever
// maxTurnAngle should not be negative
float turnToward(float current, float goal, float maxTurnAngle);
vec2 reflect(const vec2& in, const vec2& normal);

float getRandomf01();
float getRandomf(float minInclusive, float maxInclusive);
int32_t getRandomInt32(int32_t minInclusive, int32_t maxInclusive);
int32_t getRandomInt32LessThan(int32_t maxNotInclusive);
bool checkRandomChance(float chanceForSuccess);

template<class VectorType>
float dotProduct(const VectorType& a, const VectorType& b) {
  return a.dot(b);
}
//--------------------------- ranging, clamping ------------------------------------------------------------

// QA: constexpr with link error
int rounding(float in);
float roundingf(float in);

int ceiling(float in);
int clamp(int v, int min, int max);
float clampf(float v, float min, float max);
float clampf01(float v);
float clampfInAbs1(float v);
vec2 clamp(const vec2& v, vec2 min, vec2 max);
ivec2 clamp(const ivec2& v, ivec2 min, ivec2 max);

float getFraction(float v, float start, float end);

float rangeMapf(float v, float inStart, float inEnd, float outStart, float outEnd);

template<typename T>
T rangeMap(const T& v, const T& inStart, const T& inEnd, const T& outStart, const T& outEnd) {
  if (inStart == inEnd) {
    return (outStart + outEnd) * 0.5f;
  }

  vec2 inRange = inEnd - inStart,
    outRange = outEnd - outStart,
    inFromStart = v - inStart,
    fractionInRange = inFromStart / inRange;

  vec2 outFromStart = fractionInRange * outRange;

  return outFromStart + outStart;
};


//-------------------------- interpolation --------------------------------------------------

float	smoothStart2(float t); // 2nd-degree smooth start (a.k.a. “quadratic ease in”)
float	smoothStart3(float t); // 3rd-degree smooth start (a.k.a. “cubic ease in”)
float	smoothStart4(float t); // 4th-degree smooth start (a.k.a. “quartic ease in”)
float	smoothStop2(float t); // 2nd-degree smooth start (a.k.a. “quadratic ease out”)
float	smoothStop3(float t); // 3rd-degree smooth start (a.k.a. “cubic ease out”)
float	smoothStop4(float t); // 4th-degree smooth start (a.k.a. “quartic ease out”)
float	smoothStep3(float t); // 3rd-degree smooth start/stop (a.k.a. “smoothstep”)

// linear interpolate
inline float lerpf(float from, float to, float fraction);

template<typename T>
T lerp(const T& from, const T& to, float fraction) {
  return from * (1.f - fraction) + to * fraction;
};

float lerp(float from, float to, float fraction);
const vec2 lerp(const vec2& from, const vec2& to, float fraction);
const FloatRange lerp(const FloatRange& from, const FloatRange& to, float fraction);
const aabb2 lerp(const aabb2& from, const aabb2& to, float fraction);
const Disc2 lerp(const Disc2& from, const Disc2& to, float fraction);

int lerp(int from, int to, float fraction);
unsigned char lerp(unsigned char from, unsigned char to, float fraction);
const ivec2 lerp(const ivec2& from, const ivec2& to, float fraction);
const IntRange lerp(const IntRange& from, const IntRange& to, float fraction);
const Rgba lerp(const Rgba& from, const Rgba& to, float fraction);


//---------------------------- Bitwise operation --------------------------------------------
bool areBitsSet(unsigned char flag8, unsigned char mask);
bool areBitsSet(unsigned int flag32, unsigned int mask);
void setBits(unsigned char& flag8, unsigned char mask);
void setBits(unsigned int& flag32, unsigned int mask);
void clearBits(unsigned char& flag8, unsigned char mask);
void clearBits(unsigned int& flag32, unsigned int mask);


#pragma once

#define PI (3.1415926535897932384626433832795f)
#include <Math.h>
class FloatRange;
class AABB2;
class IntVector2;
class IntRange;
class Rgba;
typedef int int32_t;
class Vector2;
class Disc2;

float convertRadiansToDegrees (float radians);
float convertDegreesToRadians (float degrees);
float cosDegrees (float degrees);
float sinDegrees (float degrees);
float atan2Degree(float y, float x);

float getSquaredDistance(const Vector2& a, const Vector2& b);
float getDistance(const Vector2& a, const Vector2& b);
float getAngularDisplacement(float startDegrees, float endDegrees);

// it should work for all different scales, no matter degree or radian or whatever
// maxTurnAngle should not be negative
float turnToward(float current, float goal, float maxTurnAngle);
Vector2 reflect(const Vector2& in, const Vector2& normal);

float getRandomf01();
float getRandomf(float minInclusive, float maxInclusive);
int32_t getRandomInt32(int32_t minInclusive, int32_t maxInclusive);
int32_t getRandomInt32LessThan(int32_t maxNotInclusive);
bool checkRandomChance(float chanceForSuccess);

//--------------------------- ranging, clamping ------------------------------------------------------------

// QA: constexpr with link error
int rounding(float in);
float roundingf(float in);

int clamp(int v, int min, int max);
float clampf(float v, float min, float max);
float clampf01(float v);
float clampfInAbs1(float v);

float getFraction(float v, float start, float end);

float rangeMapf(float v, float inStart, float inEnd, float outStart, float outEnd);

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
T lerp(const T& from, const T& to, float fraction);
float lerp(float from, float to, float fraction);
const Vector2 lerp(const Vector2& from, const Vector2& to, float fraction);
const FloatRange lerp(const FloatRange& from, const FloatRange& to, float fraction);
const AABB2 lerp(const AABB2& from, const AABB2& to, float fraction);
const Disc2 lerp(const Disc2& from, const Disc2& to, float fraction);

int lerp(int from, int to, float fraction);
unsigned char lerp(unsigned char from, unsigned char to, float fraction);
const IntVector2 lerp(const IntVector2& from, const IntVector2& to, float fraction);
const IntRange lerp(const IntRange& from, const IntRange& to, float fraction);
const Rgba lerp(const Rgba& from, const Rgba& to, float fraction);


//---------------------------- Bitwise operation --------------------------------------------
bool areBitsSet(unsigned char flag8, unsigned char mask);
bool areBitsSet(unsigned int flag32, unsigned int mask);
void setBits(unsigned char& flag8, unsigned char mask);
void setBits(unsigned int& flag32, unsigned int mask);
void clearBits(unsigned char& flag8, unsigned char mask);
void clearBits(unsigned int& flag32, unsigned int mask);


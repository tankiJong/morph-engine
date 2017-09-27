#pragma once

#define PI (3.1415926535897932384626433832795f)
#include <Math.h>
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

float getRandomf01();
float getRandomf(float minInclusive, float maxInclusive);
int32_t getRandomInt32(int32_t minInclusive, int32_t maxInclusive);
int32_t getRandomInt32LessThan(int32_t maxNotInclusive);
bool checkRandomChance(float chanceForSuccess);

//--------------------------- ranging, clamping ------------------------------------------------------------

int rounding(float in); // TODO: warp math.h
float roundingf(float in);

int clamp(int v, int min, int max);
float clampf(float v, float min, float max);
float clampf01(float v);
float clampfInAbs1(float v);

float getFraction(float v, float start, float end);

float rangeMapf(float v, float inStart, float inEnd, float outStart, float outEnd);

//-------------------------- interpolation --------------------------------------------------

// linear interpolate
float lerpf(float from, float to, float fraction);


//---------------------------- Bitwise operation --------------------------------------------
bool areBitsSet(unsigned char flag8, unsigned char mask);
bool areBitsSet(unsigned int flag32, unsigned int mask);
void setBits(unsigned char& flag8, unsigned char mask);
void setBits(unsigned int& flag32, unsigned int mask);
void clearBits(unsigned char& flag8, unsigned char mask);
void clearBits(unsigned int& flag32, unsigned int mask);


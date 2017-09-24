#pragma once

#include <stdint.h>
#define PI (3.1415926535897932384626433832795f)

class Vector2;
class Disc2;
extern "C" float sqrtf(float);
extern "C" float atan2f(float y, float x);
float convertRadiansToDegrees (float radians);
float convertDegreesToRadians (float degrees);
float cosDegrees (float degrees);
float sinDegrees (float degrees);
float atan2Degree(float y, float x);
float getRandomFromZerotoOne();
float getRandomInRange(float minInclusive, float maxInclusive);
int32_t getRandomInt32InRange(int32_t minInclusive, int32_t maxInclusive);

float getSquaredDistance(const Vector2& a, const Vector2& b);
float getDistance(const Vector2& a, const Vector2& b);

float clampf(float value, float min, float max);
float clampf01(float value);
float rangeMapf(float v, float inStart, float inEnd, float outStart, float outEnd);
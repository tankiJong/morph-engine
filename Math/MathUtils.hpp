#pragma once

#include <stdint.h>
#define PI (3.1415926535897932384626433832795f)

class Vector2;
class Disc2;
float convertRadiansToDegrees (float radians);
float convertDegreesToRadians (float degrees);
float cosDegrees (float degrees);
float sinDegrees (float degrees);
float getRandomFromZerotoOne();
float getRandomInRange(float minInclusive, float maxInclusive);
int32_t getRandomInt32InRange(int32_t minInclusive, int32_t maxInclusive);

float getSquaredDistance(const Vector2& a, const Vector2& b);
float getDistance(const Vector2& a, const Vector2& b);

float clampf(float value, float min, float max);

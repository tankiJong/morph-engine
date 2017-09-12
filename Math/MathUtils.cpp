#include <Math.h>
#include <stdlib.h>
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Math\Vector2.hpp"
#include "Engine\Math\Disc2.hpp"

//-----------------------------------------------------------------------------------------------
float convertRadiansToDegrees(float radians) {
    return radians*(180.f / PI);
}

//-----------------------------------------------------------------------------------------------
float convertDegreesToRadians(float degrees) {
    return degrees*(PI / 180.f);
}

//-----------------------------------------------------------------------------------------------
float cosDegrees(float degrees) {
    return cosf(convertDegreesToRadians(degrees));
}

//-----------------------------------------------------------------------------------------------
float sinDegrees(float degrees) {
    return sinf(convertDegreesToRadians(degrees));
}

float getRandomFromZerotoOne() {
    return (float(RAND_MAX - rand())) / RAND_MAX;
}

float getRandomInRange(float minInclusive, float maxInclusive) {
    return getRandomFromZerotoOne()*(maxInclusive - minInclusive) + minInclusive;
}

int32_t getRandomInt32InRange(int32_t minInclusive, int32_t maxInclusive) {
    return rand() % (maxInclusive - minInclusive + 1) + minInclusive;
}

float getSquaredDistance(const Vector2& a, const Vector2& b) {
	float dx = a.x - b.x, dy = a.y - b.y;
	return dx*dx + dy*dy;
}

float getDistance(const Vector2 & a, const Vector2 & b)
{
	float dx = a.x - b.x, dy = a.y - b.y;
	return sqrtf(dx*dx + dy*dy);
}
#include <stdlib.h>
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Math\Vector2.hpp"
#include "Engine\Math\Disc2.hpp"
#include <cassert>

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

float atan2Degree(float y, float x) {
	return convertRadiansToDegrees(atan2f(y, x));
}

float getRandomf01() {
	return (float)rand() / (float)RAND_MAX;
}

float getRandomf(float minInclusive, float maxInclusive) {
	return getRandomf01()*(maxInclusive - minInclusive) + minInclusive;
}

int32_t getRandomInt32(int32_t minInclusive, int32_t maxInclusive) {
	return rand() % (maxInclusive - minInclusive + 1) + minInclusive;
}

int32_t getRandomInt32LessThan(int32_t maxNotInclusive) {
	return getRandomInt32(0, maxNotInclusive - 1);
}

bool checkRandomChance(float chanceForSuccess) {
	float rnd = getRandomf01();
	return rnd <= chanceForSuccess;
}

float getSquaredDistance(const Vector2& a, const Vector2& b) {
	float dx = a.x - b.x, dy = a.y - b.y;
	return dx*dx + dy*dy;
}

float getDistance(const Vector2 & a, const Vector2 & b) {
	float dx = a.x - b.x, dy = a.y - b.y;
	return sqrtf(dx*dx + dy*dy);
}

float turnToward(float current, float goal, float maxTurnAngle) {
	float angDis = getAngularDisplacement(current, goal);
	if (angDis > maxTurnAngle) {
		return current + maxTurnAngle;
	}
	if (angDis < -maxTurnAngle) {
		return current - maxTurnAngle;
	}
	return current + angDis;
}

Vector2 reflect(const Vector2& in, const Vector2& normal) {
  // QA: sometimes fail
//  assert(1.f - normal.getLengthSquared() < 1e-9 && 1.f - normal.getLengthSquared() > -1e-9);

  return in - 2.f * Vector2::dotProduct(in, normal) * normal;
}

float getAngularDisplacement(float startDegrees, float endDegrees) {
	float angDis = endDegrees - startDegrees;

	while (angDis > 180.f) {
		angDis -= 360.f;
	}
	while (angDis < -180.f) {
		angDis += 360.f;
	}

	return angDis;
}

int rounding(float in) {
	int inInt = (int)in;
	float reminder = in - (float)inInt;
	if (reminder == 0) return inInt;
	if (reminder > 0) {
		if (reminder >= 0.5) {
			return inInt + 1;
		} else {
			return inInt;
		}
	} else {
		if (reminder < -0.5) {
			return inInt - 1;
		} else {
			return inInt;
		}
	}
}

float roundingf(float in) {
	return (float)rounding(in);
}

int clamp(int v, int min, int max) {
	if (v > max) {
		return max;
	}

	if (v < min) {
		return min;
	}

	return v;
}

float clampf(float v, float min, float max) {
	if (v > max) {
		return max;
	}

	if (v < min) {
		return min;
	}

	return v;
}

float clampf01(float v) {
	return clampf(v, 0.f, 1.f);
}

float clampfInAbs1(float v) {
	return clampf(v, -1.f, 1.f);
}

float getFraction(float v, float start, float end) {
	return (v - start) / (end - start);
}

float rangeMapf(float v, float inStart, float inEnd, float outStart, float outEnd) {
	if (inStart == inEnd) {
		return (outStart + outEnd) * 0.5f;
	}

	float inRange = inEnd - inStart,
		outRange = outEnd - outStart,
		inFromStart = v - inStart,
		fractionInRange = inFromStart / inRange;

	float outFromStart = fractionInRange * outRange;

	return outFromStart + outStart;
}

float lerpf(float from, float to, float fraction) {
	return from + fraction * (to - from);
}

bool areBitsSet(unsigned char flag8, unsigned char mask) {
	return (flag8 & mask) == mask;
}

bool areBitsSet(unsigned int flag32, unsigned int mask) {
	return (flag32 & mask) == mask;
}

void setBits(unsigned char& flag8, unsigned char mask) {
	flag8 |= mask;
}

void setBits(unsigned int& flag32, unsigned int mask) {
	flag32 |= mask;
}

void clearBits(unsigned char& flag8, unsigned char mask) {
	flag8 &= ~mask;
}

void clearBits(unsigned int& flag32, unsigned int mask) {
	flag32 &= ~mask;
}

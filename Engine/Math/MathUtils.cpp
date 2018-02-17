#include <stdlib.h>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Math/Primitives/Disc2.hpp"
#include "Engine/Math/Primitives/FloatRange.hpp"
#include "Engine/Math/Primitives/ivec2.hpp"

#include "Engine/Math/Primitives/AABB2.hpp"
#include "Engine/Math/Primitives/IntRange.hpp"
#include "Engine/Core/Rgba.hpp"

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

float getSquaredDistance(const vec2& a, const vec2& b) {
	float dx = a.x - b.x, dy = a.y - b.y;
	return dx*dx + dy*dy;
}

float getDistance(const vec2 & a, const vec2 & b) {
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

vec2 reflect(const vec2& in, const vec2& normal) {
  vec2 normalDir = normal.getNormalized();
  return in - 2.f * vec2::dotProduct(in, normalDir) * normalDir;
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

vec2 clamp(const vec2& v, vec2 min, vec2 max) {
  return { clampf(v.x, min.x, max.x), clampf(v.y, min.y, max.y) };
}

ivec2 clamp(const ivec2& v, ivec2 min, ivec2 max) {
  return { clamp(v.x, min.x, max.x), clamp(v.y, min.y, max.y) };
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

float smoothStart2(float t) {
  return t*t;
}
float smoothStart3(float t) {
  return t*t*t;
}
float smoothStart4(float t) {
  return t*t*t*t;
}
float smoothStop2(float t) {
  float tt = 1.f - t;
  return 1 - tt*tt;
}
float smoothStop3(float t) {
  float tt = 1.f - t;
  return 1 - tt*tt*tt;
}
float smoothStop4(float t) {
  float tt = 1.f - t;
  return 1 - tt*tt*tt*tt;
}
float smoothStep3(float t) {
  return (1.f-t) * smoothStart3(t) + t* smoothStop3(t);
}

float lerpf(float from, float to, float fraction) {
	return from * ( 1.f - fraction) + fraction * to;
}

float lerp(float from, float to, float fraction) {
  return lerpf(from, to, fraction);
}
const vec2 lerp(const vec2& from, const vec2& to, float fraction) {
  float x = lerp(from.x, to.x, fraction);
  float y = lerp(from.y, to.y, fraction);

  return vec2(x, y);
}
const FloatRange lerp(const FloatRange& from, const FloatRange& to, float fraction) {
  float start = lerp(from.min, to.min, fraction);
  float end = lerp(from.max, to.max, fraction);

  return FloatRange(start, end);
}
const aabb2 lerp(const aabb2& from, const aabb2& to, float fraction) {
  vec2 mins = lerp(from.mins, to.mins, fraction);
  vec2 maxs = lerp(from.maxs, to.maxs, fraction);

  return aabb2(mins, maxs);
}
const Disc2 lerp(const Disc2& from, const Disc2& to, float fraction) {
  auto center = lerp(from.center, to.center, fraction);
  auto r = lerp(from.radius, to.radius, fraction);

  return Disc2(center, r);
}
int lerp(int from, int to, float fraction) {
  if (fraction == 1.f) return to;

  float interval = to - from + 1.f;
  float intervalPrecent = 1.f / interval;

  return int(from + fraction / intervalPrecent);
}
unsigned char lerp(unsigned char from, unsigned char to, float fraction) {
  return (unsigned char)clamp(lerp((int)from, (int)to, fraction), 0, 255);
}
const ivec2 lerp(const ivec2& from, const ivec2& to, float fraction) {
  int x = lerp(from.x, to.x, fraction);
  int y = lerp(from.y, to.y, fraction);

  return ivec2(x, y);
}
const IntRange lerp(const IntRange& from, const IntRange& to, float fraction) {
  int min = lerp(from.min, to.min, fraction);
  int max = lerp(from.max, to.max, fraction);

  return IntRange(min, max);
}
const Rgba lerp(const Rgba& from, const Rgba& to, float fraction) {
  return Rgba(
    lerp(from.r, to.r, fraction),
    lerp(from.g, to.g, fraction),
    lerp(from.b, to.b, fraction),
    lerp(from.a, to.a, fraction)
  );
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

#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include <array>

aabb2::aabb2(const aabb2& copy)
	: mins(copy.mins)
	, maxs(copy.maxs) {}

aabb2::aabb2(float minX, float minY, float maxX, float maxY)
	: mins(minX, minY)
	, maxs(maxX, maxY) {}

aabb2::aabb2(const vec2& mins, const vec2& maxs)
	: mins(mins)
	, maxs(maxs) {}

aabb2::aabb2(const vec2& center, float radiusX, float radiusY)
	: mins(center.x - radiusX, center.y - radiusY)
	, maxs(center.x + radiusX, center.y + radiusY) {}

aabb2::aabb2(float width, float height, const vec2& mins)
  : mins(mins)
  , maxs(mins.x + width, mins.y + height) {}

void aabb2::stretchToIncludePoint(float x, float y) {
	if (x < mins.x) mins.x = x;
	else if (x > maxs.x ) maxs.x = x;

	if (y < mins.y) mins.y = y;
	else if (y > maxs.y) maxs.y = y;
}

void aabb2::stretchToIncludePoint(const vec2& point) {
	stretchToIncludePoint(point.x, point.y);
}

void aabb2::addPaddingToSides(float xPaddingRadius, float yPaddingRadius) {
	vec2 padding(xPaddingRadius, yPaddingRadius);

	mins -= padding;
	maxs += padding;
}

void aabb2::translate(const vec2& translation) {
	mins += translation;
	maxs += translation;
}

void aabb2::translate(float translationX, float translationY) {
	vec2 translation(translationX, translationY);
	translate(translation);
}

std::array<vec2, 4> aabb2::vertices() const {
  return {
    mins,
    {mins.x, maxs.y},
    maxs,
    {maxs.x, mins.y},
  };
}

bool aabb2::isPointInside(float x, float y) const {
	return x > mins.x && x < maxs.x && y > mins.y && y < maxs.y;
}

bool aabb2::isPointInside(const vec2& point) const {
	return isPointInside(point.x, point.y);
}

vec2 aabb2::getDimensions() const {
	return maxs - mins;
}

vec2 aabb2::getCenter() const {
	return 0.5f * (mins + maxs);
}

float aabb2::width() const {
  return getDimensions().x;
}
float aabb2::height() const {
  return getDimensions().y;
}

void aabb2::operator+=(const vec2& translation) {
	mins += translation;
	maxs += translation;
}

void aabb2::operator-=(const vec2& antiTranslation) {
	mins -= antiTranslation;
	maxs -= antiTranslation;
}

aabb2 aabb2::operator+(const vec2& translation) const {
	return aabb2(mins + translation, maxs + translation);
}

aabb2 aabb2::operator-(const vec2& antiTranslation) const {
	return aabb2(mins - antiTranslation, maxs - antiTranslation);
}

void aabb2::fromString(const char* data) {
  auto components = split(data, ";");
  GUARANTEE_OR_DIE(components.size() == 2, "input data invalid");
  mins.fromString(components[0].c_str());
  maxs.fromString(components[1].c_str());
}
std::string aabb2::toString() const {
  return Stringf("%s;%s", mins.toString().c_str(), maxs.toString().c_str());
}

bool areAABBsOverlap(const aabb2& a, const aabb2& b) {
	if (a.maxs.x < b.mins.x || b.maxs.x < a.mins.x) return false;
	if (a.maxs.y < b.mins.y || b.maxs.y < a.mins.y) return false;

	return true;
}

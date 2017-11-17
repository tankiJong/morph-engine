#pragma once

#include "Engine/Math/AABB2.hpp"



AABB2::AABB2(const AABB2& copy)
	: mins(copy.mins)
	, maxs(copy.maxs) {}

AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	: mins(minX, minY)
	, maxs(maxX, maxY) {}

AABB2::AABB2(const Vector2& mins, const Vector2& maxs)
	: mins(mins)
	, maxs(maxs) {}

AABB2::AABB2(const Vector2& center, float radiusX, float radiusY)
	: mins(center.x - radiusX, center.y - radiusY)
	, maxs(center.x + radiusX, center.y + radiusY) {}

void AABB2::stretchToIncludePoint(float x, float y) {
	if (x < mins.x) mins.x = x;
	else if (x > maxs.x ) maxs.x = x;

	if (y < mins.y) mins.y = y;
	else if (y > maxs.y) maxs.y = y;
}

void AABB2::stretchToIncludePoint(const Vector2& point) {
	stretchToIncludePoint(point.x, point.y);
}

void AABB2::addPaddingToSides(float xPaddingRadius, float yPaddingRadius) {
	Vector2 padding(xPaddingRadius, yPaddingRadius);

	mins -= padding;
	maxs += padding;
}

void AABB2::translate(const Vector2& translation) {
	mins += translation;
	maxs += translation;
}

void AABB2::translate(float translationX, float translationY) {
	Vector2 translation(translationX, translationY);
	translate(translation);
}

bool AABB2::isPointInside(float x, float y) const {
	return x > mins.x && x < maxs.x && y > mins.y && y < maxs.y;
}

bool AABB2::isPointInside(const Vector2& point) const {
	return isPointInside(point.x, point.y);
}

Vector2 AABB2::getDimensions() const {
	return maxs - mins;
}

Vector2 AABB2::getCenter() const {
	return 0.5f * (mins + maxs);
}

float AABB2::width() const {
  return getDimensions().x;
}
float AABB2::height() const {
  return getDimensions().y;
}

void AABB2::operator+=(const Vector2& translation) {
	mins += translation;
	maxs += translation;
}

void AABB2::operator-=(const Vector2& antiTranslation) {
	mins -= antiTranslation;
	maxs -= antiTranslation;
}

AABB2 AABB2::operator+(const Vector2& translation) const {
	return AABB2(mins + translation, maxs + translation);
}

AABB2 AABB2::operator-(const Vector2& antiTranslation) const {
	return AABB2(mins - antiTranslation, maxs - antiTranslation);
}

bool areAABBsOverlap(const AABB2& a, const AABB2& b) {
	if (a.maxs.x < b.mins.x || b.maxs.x < a.mins.x) return false;
	if (a.maxs.y < b.mins.y || b.maxs.y < a.mins.y) return false;

	return true;
}
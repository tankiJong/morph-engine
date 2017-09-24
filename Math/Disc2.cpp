#include "Engine\Math\Disc2.hpp"
#include "Engine\Math\MathUtils.hpp"

Disc2::Disc2(const Disc2 & copyFrom) {
	center = copyFrom.center;
	radius = copyFrom.radius;
}

Disc2::Disc2(float initialX, float initialY, float initialRadius)
	 : center(initialX, initialY)
     , radius(initialRadius) {}

Disc2::Disc2(const Vector2 & initialCenter, float initialRadius)
     : center(initialCenter)
     , radius(initialRadius) {}

void Disc2::stretchToIncludePoint(float x, float y) {
	const float squaredDis = getSquaredDistance(Vector2(x, y), center);
	if ( squaredDis <= radius*radius) {
		return;
	}

	radius = sqrtf(squaredDis);
}

void Disc2::stretchToIncludePoint(const Vector2 & point) {
	stretchToIncludePoint(point.x, point.y);
}

void Disc2::addPaddingToRadius(float paddingRadius) {
	radius += paddingRadius;
}

void Disc2::translate(const Vector2 & translation) {
	center += translation;
}

void Disc2::translate(float translationX, float translationY) {
	center += Vector2(translationX, translationY);
}

bool Disc2::isPointInside(float x, float y) const {
	return getSquaredDistance(center, Vector2(x, y)) < radius*radius;
}

bool Disc2::isPointInside(const Vector2 & point) const {
	return isPointInside(point.x, point.y);
}

void Disc2::operator+=(const Vector2 & translation) {
	translate(translation);
}

void Disc2::operator-=(const Vector2 & antiTranslation) {
	translate(antiTranslation*(-1.f));
}

Disc2 Disc2::operator+(const Vector2 & translation) const {
	return Disc2(center + translation, radius);
}

Disc2 Disc2::operator-(const Vector2 & antiTranslation) const {
	return Disc2(center - antiTranslation, radius);
}

bool areDiscsOverlap(const Disc2& a, const Disc2& b)
{
    	float radius = a.radius + b.radius;
	float sqDis = getSquaredDistance(a.center, b.center);
	if(sqDis >= radius * radius) return 0;
	else return 1;
}

bool areDiscsOverlap(const Vector2 & aCenter, float aRadius, const Vector2 & bCenter, float bRadius)
{
	float radius = aRadius + bRadius;
	return getSquaredDistance(aCenter, bCenter) < radius * radius;
}

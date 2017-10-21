
#include "Engine\Math\Vector2.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "IntVector2.hpp"
#include <initializer_list>

const Vector2 Vector2::zero = Vector2(0.f, 0.f);

//-----------------------------------------------------------------------------------------------
Vector2::Vector2 (const Vector2& copy)
    : x (copy.x)
    , y (copy.y) {}

Vector2::Vector2(const IntVector2& copyFrom)
    : x((float)copyFrom.x)
    , y((float)copyFrom.y) {
  
}

//-----------------------------------------------------------------------------------------------
Vector2::Vector2 (float initialX, float initialY)
    : x (initialX)
    , y (initialY) {}

Vector2::Vector2(std::initializer_list<float> list) {
  auto begin = list.begin();
  x = *begin++;
  y = *begin;
}

//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator + (const Vector2& vecToAdd) const {
    return Vector2 (this->x + vecToAdd.x, this->y + vecToAdd.y);
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator-(const Vector2& vecToSubtract) const {
    return Vector2 (this->x - vecToSubtract.x, this->y - vecToSubtract.y);
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator*(float uniformScale) const {
    return Vector2 (this->x*uniformScale, this->y*uniformScale);
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator/(float inverseScale) const {
    return Vector2 (this->x*(1.f / inverseScale), this->y*(1.f / inverseScale));
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator+=(const Vector2& vecToAdd) {
    x = x + vecToAdd.x;
    y = y + vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator-=(const Vector2& vecToSubtract) {
    x = x - vecToSubtract.x;
    y = y - vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator*=(const float uniformScale) {
    x = x * uniformScale;
    y = y * uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator/=(const float uniformDivisor) {
    x = x / uniformDivisor;
    y = y / uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator=(const Vector2& copyFrom) {
    x = copyFrom.x;
    y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vector2 operator*(float uniformScale, const Vector2& vecToScale) {
    return Vector2 (vecToScale.x * uniformScale, vecToScale.y * uniformScale);
}

float Vector2::dotProduct(const Vector2 & a, const Vector2 & b) {
  return a.x*b.x + a.y*b.y;
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator==(const Vector2& compare) const {
    return x == compare.x && y == compare.y;
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator!=(const Vector2& compare) const {
    return x != compare.x || y != compare.y;
}

float Vector2::getLength() const {
	return sqrtf(x*x + y*y);
}

float Vector2::getLengthSquared() const {
	return x*x+y*y;
}

float Vector2::normalizeAndGetLength() {
	float magnitude = getLength();
	x = x / magnitude;
	y = y / magnitude;
	return magnitude;
}

Vector2 Vector2::getNormalized() const {
	float magnitude = getLength();
	return Vector2(x / magnitude, y / magnitude);
}

float Vector2::getOrientationDegrees() const {
	return convertRadiansToDegrees(atan2f(y, x));

}

Vector2 Vector2::makeDirectionAtDegrees(float degrees) {
	return Vector2(cosDegrees(degrees), sinDegrees(degrees));
}


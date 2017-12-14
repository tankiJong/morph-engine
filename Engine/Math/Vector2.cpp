
#include "IntVector2.hpp"
#include <initializer_list>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const Vector2 Vector2::zero(0.f, 0.f);
const Vector2 Vector2::top(0, 1.f);
const Vector2 Vector2::down(0, -1.f);
const Vector2 Vector2::left(-1.f, 0);
const Vector2 Vector2::right(1.f, 0);
//-----------------------------------------------------------------------------------------------
Vector2::Vector2 (const Vector2& copy)
    : x (copy.x)
    , y (copy.y) {}

Vector2::Vector2(const IntVector2& copyFrom)
    : x((float)copyFrom.x)
    , y((float)copyFrom.y) {
  
}

Vector2::Vector2(const Vector3& copyFrom) 
  : x(copyFrom.x)
  , y(copyFrom.y) {}

//-----------------------------------------------------------------------------------------------
Vector2::Vector2 (float initialX, float initialY)
    : x (initialX)
    , y (initialY) {}

Vector2::Vector2(std::initializer_list<float> list) {
  auto begin = list.begin();
  x = *begin++;
  y = *begin;
}

Vector2::Vector2(const char* str) {
  fromString(str);
}

void Vector2::fromString(const char* data) {
  auto raw = split(data, " ,");
  GUARANTEE_OR_DIE(raw.size() == 2, "illegal input string to parse");

  x = parse<float>(raw[0]);
  y = parse<float>(raw[1]);
}

std::string Vector2::toString() const {
  return Stringf("%f,%f", x, y);
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

void Vector2::operator=(const Vector3& copyFrom) {
  x = copyFrom.x;
  y = copyFrom.y;
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

float Vector2::dot(const Vector2& a, const Vector2& b) {
  return a.dot(b);
}

float Vector2::angle(const Vector2& a, const Vector2& b) {
  return getAngularDisplacement(a.getOrientationDegrees(), b.getOrientationDegrees());
}

const Vector2 projectTo(const Vector2& vectorToProject, const Vector2& projectOnto) {


  return (vectorToProject.dot(projectOnto) / projectOnto.getLengthSquared())
        * projectOnto;
}

const Vector2 transform(const Vector2& originalVector, 
                        const Vector2& fromX, const Vector2& fromY, 
                        const Vector2& toI, const Vector2& toJ) {
  float nx = originalVector.x * fromX.x + originalVector.y * fromY.x;
  float ny = originalVector.x * fromX.y + originalVector.y * fromY.y;

  Vector2 vecInStandard(nx, ny);

  float i = vecInStandard.dot(toI) / toI.getLengthSquared();
  float j = vecInStandard.dot(toJ) / toJ.getLengthSquared();

  return Vector2(i, j);
}

const Vector2 transToBasis(const Vector2& originalVector, 
                           const Vector2& toBasisI, const Vector2& toBasisJ) {
  return transform(originalVector, Vector2::right, Vector2::top, toBasisI, toBasisJ);
}

const Vector2 transFromBasis(const Vector2& originalVector, 
                             const Vector2& fromBasisI, const Vector2& fromBasisJ) {
  return transform(originalVector, fromBasisI, fromBasisJ, Vector2::right, Vector2::top);
}

void decompose(const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ,
  Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ) {
  out_vectorAlongI = projectTo(originalVector, newBasisI);
  out_vectorAlongJ = projectTo(originalVector, newBasisJ);
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
  if(magnitude == 0) {
    return 0;
  }

  float rmagnitude = 1.f / magnitude;
	x *= rmagnitude;
	y *= rmagnitude;
	return magnitude;
}

Vector2 Vector2::getNormalized() const {
	float magnitude = getLength();
  if (magnitude == 0) return Vector2::zero;
  magnitude = 1.f / magnitude;
	return Vector2(x * magnitude, y * magnitude);
}

float Vector2::getOrientationDegrees() const {
	return convertRadiansToDegrees(atan2f(y, x));
}

float Vector2::dot(const Vector2& another) const {
  return dotProduct(*this, another);
}

void Vector2::setAngle(float degree) {
  float length = getLength();
  x = cosDegrees(degree) * length;
  y = sinDegrees(degree) * length;
}

Vector2 Vector2::makeDirectionAtDegrees(float degrees) {
	return Vector2(cosDegrees(degrees), sinDegrees(degrees));
}


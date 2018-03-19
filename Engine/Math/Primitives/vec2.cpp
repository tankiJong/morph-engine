
#include "Ivec2.hpp"
#include <initializer_list>
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Math/Primitives/uvec2.hpp"

const vec2 vec2::one(1.f, 1.f);
const vec2 vec2::zero(0.f, 0.f);
const vec2 vec2::top(0.f, 1.f);
const vec2 vec2::down(0.f, -1.f);
const vec2 vec2::left(-1.f, 0.f);
const vec2 vec2::right(1.f, 0.f);
//-----------------------------------------------------------------------------------------------
vec2::vec2 (const vec2& copy)
    : x (copy.x)
    , y (copy.y) {}

vec2::vec2(const ivec2& copyFrom)
    : x((float)copyFrom.x)
    , y((float)copyFrom.y) {
  
}

vec2::vec2(const uvec2& copyFrom)
  : x((float)copyFrom.x)
  , y((float)copyFrom.y) {

}


//-----------------------------------------------------------------------------------------------
vec2::vec2 (float initialX, float initialY)
    : x (initialX)
    , y (initialY) {}

vec2::vec2(const char* str) {
  fromString(str);
}

void vec2::fromString(const char* data) {
  auto raw = split(data, " ,");
  GUARANTEE_OR_DIE(raw.size() == 2, "illegal input string to parse");

  x = parse<float>(raw[0]);
  y = parse<float>(raw[1]);
}

std::string vec2::toString() const {
  return Stringf("%f,%f", x, y);
}

//-----------------------------------------------------------------------------------------------
vec2 vec2::operator + (const vec2& vecToAdd) const {
    return vec2 (this->x + vecToAdd.x, this->y + vecToAdd.y);
}


//-----------------------------------------------------------------------------------------------
vec2 vec2::operator-(const vec2& vecToSubtract) const {
    return vec2 (this->x - vecToSubtract.x, this->y - vecToSubtract.y);
}

vec2 vec2::operator-() const {
  return { -x, -y };
}

//-----------------------------------------------------------------------------------------------
vec2 vec2::operator*(float uniformScale) const {
    return vec2 (this->x*uniformScale, this->y*uniformScale);
}


//-----------------------------------------------------------------------------------------------
vec2 vec2::operator/(float inverseScale) const {
    return vec2 (this->x*(1.f / inverseScale), this->y*(1.f / inverseScale));
}

vec2 vec2::operator*(const vec2& rhs) const {
  return { rhs.x*x, rhs.y*y };
}
vec2 vec2::operator/(const vec2& rhs) const {
  return { x/rhs.x, y/rhs.y };
}

void vec2::operator=(const vec3& copyFrom) {
  x = copyFrom.x;
  y = copyFrom.y;
}

//-----------------------------------------------------------------------------------------------
void vec2::operator+=(const vec2& vecToAdd) {
    x = x + vecToAdd.x;
    y = y + vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void vec2::operator-=(const vec2& vecToSubtract) {
    x = x - vecToSubtract.x;
    y = y - vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void vec2::operator*=(const float uniformScale) {
    x = x * uniformScale;
    y = y * uniformScale;
}


//-----------------------------------------------------------------------------------------------
void vec2::operator/=(const float uniformDivisor) {
    x = x / uniformDivisor;
    y = y / uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void vec2::operator=(const vec2& copyFrom) {
    x = copyFrom.x;
    y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const vec2 operator*(float uniformScale, const vec2& vecToScale) {
    return vec2 (vecToScale.x * uniformScale, vecToScale.y * uniformScale);
}

float vec2::dotProduct(const vec2 & a, const vec2 & b) {
  return a.x*b.x + a.y*b.y;
}

float vec2::dot(const vec2& a, const vec2& b) {
  return a.dot(b);
}

float vec2::angle(const vec2& a, const vec2& b) {
  return getAngularDisplacement(a.getOrientationDegrees(), b.getOrientationDegrees());
}

const vec2 projectTo(const vec2& vectorToProject, const vec2& projectOnto) {


  return (vectorToProject.dot(projectOnto) / projectOnto.getLengthSquared())
        * projectOnto;
}

const vec2 transform(const vec2& originalVector, 
                        const vec2& fromX, const vec2& fromY, 
                        const vec2& toI, const vec2& toJ) {
  float nx = originalVector.x * fromX.x + originalVector.y * fromY.x;
  float ny = originalVector.x * fromX.y + originalVector.y * fromY.y;

  vec2 vecInStandard(nx, ny);

  float i = vecInStandard.dot(toI) / toI.getLengthSquared();
  float j = vecInStandard.dot(toJ) / toJ.getLengthSquared();

  return vec2(i, j);
}

const vec2 transToBasis(const vec2& originalVector, 
                           const vec2& toBasisI, const vec2& toBasisJ) {
  return transform(originalVector, vec2::right, vec2::top, toBasisI, toBasisJ);
}

const vec2 transFromBasis(const vec2& originalVector, 
                             const vec2& fromBasisI, const vec2& fromBasisJ) {
  return transform(originalVector, fromBasisI, fromBasisJ, vec2::right, vec2::top);
}

void decompose(const vec2& originalVector, const vec2& newBasisI, const vec2& newBasisJ,
  vec2& out_vectorAlongI, vec2& out_vectorAlongJ) {
  out_vectorAlongI = projectTo(originalVector, newBasisI);
  out_vectorAlongJ = projectTo(originalVector, newBasisJ);
}

//-----------------------------------------------------------------------------------------------
bool vec2::operator==(const vec2& compare) const {
    return x == compare.x && y == compare.y;
}


//-----------------------------------------------------------------------------------------------
bool vec2::operator!=(const vec2& compare) const {
    return x != compare.x || y != compare.y;
}

float vec2::magnitude() const {
  return sqrtf(x*x + y * y);
}

float vec2::getLength() const {
	return sqrtf(x*x + y*y);
}

float vec2::getLengthSquared() const {
	return x*x+y*y;
}

float vec2::normalizeAndGetLength() {
	float magnitude = getLength();
  if(magnitude == 0) {
    return 0;
  }

  float rmagnitude = 1.f / magnitude;
	x *= rmagnitude;
	y *= rmagnitude;
	return magnitude;
}

vec2 vec2::getNormalized() const {
	float magnitude = getLength();
  if (magnitude == 0) return vec2::zero;
  magnitude = 1.f / magnitude;
	return vec2(x * magnitude, y * magnitude);
}

float vec2::getOrientationDegrees() const {
	return convertRadiansToDegrees(atan2f(y, x));
}

float vec2::dot(const vec2& another) const {
  return dotProduct(*this, another);
}

void vec2::setAngle(float degree) {
  float length = getLength();
  x = cosDegrees(degree) * length;
  y = sinDegrees(degree) * length;
}

vec2 vec2::makeDirectionAtDegrees(float degrees) {
	return vec2(cosDegrees(degrees), sinDegrees(degrees));
}


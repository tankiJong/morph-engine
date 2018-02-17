#include "Ivec2.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Uvec2.hpp"
const ivec2 ivec2::zero = ivec2(0, 0);
const ivec2 ivec2::top = ivec2(0, 1);
const ivec2 ivec2::down = ivec2(0, -1);
const ivec2 ivec2::left = ivec2(-1, 0);
const ivec2 ivec2::right = ivec2(1, 0);

//-----------------------------------------------------------------------------------------------
ivec2::ivec2(const ivec2& copy)
	: x(copy.x)
	, y(copy.y) {}


//-----------------------------------------------------------------------------------------------
ivec2::ivec2(int initialX, int initialY)
	: x(initialX)
	, y(initialY) {}

ivec2::ivec2(const vec2& castFrom)
  : x((int)castFrom.x)
  , y((int)castFrom.y) {
}

ivec2::ivec2(const uvec2& castFrom): x((int)castFrom.x), y((int)castFrom.y) {}



//-----------------------------------------------------------------------------------------------
const ivec2 ivec2::operator + (const ivec2& vecToAdd) const {
	return ivec2(this->x + vecToAdd.x, this->y + vecToAdd.y);
}

const vec2 ivec2::operator+(const vec2& vecToAdd) const {
  return vecToAdd + vec2(*this);
}

//-----------------------------------------------------------------------------------------------
const ivec2 ivec2::operator-(const ivec2& vecToSubtract) const {
	return ivec2(this->x - vecToSubtract.x, this->y - vecToSubtract.y);
}


//-----------------------------------------------------------------------------------------------
const ivec2 ivec2::operator*(int uniformScale) const {
	return ivec2(this->x*uniformScale, this->y*uniformScale);
}


int ivec2::operator*(const ivec2& another) const {
	return x*another.x + y*another.y;
}

//-----------------------------------------------------------------------------------------------
const ivec2 ivec2::operator/(int inverseScale) const {
	return ivec2(this->x / inverseScale, this->y / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void ivec2::operator+=(const ivec2& vecToAdd) {
	x = x + vecToAdd.x;
	y = y + vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void ivec2::operator-=(const ivec2& vecToSubtract) {
	x = x - vecToSubtract.x;
	y = y - vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void ivec2::operator*=(const int uniformScale) {
	x = x * uniformScale;
	y = y * uniformScale;
}


//-----------------------------------------------------------------------------------------------
void ivec2::operator/=(const int uniformDivisor) {
	x = x / uniformDivisor;
	y = y / uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void ivec2::operator=(const ivec2& copyFrom) {
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const ivec2 operator*(int uniformScale, const ivec2& vecToScale) {
	return ivec2(vecToScale.x * uniformScale, vecToScale.y * uniformScale);
}

int dotProduct(const ivec2 & a, const ivec2 & b) {
	return a*b;
}


//-----------------------------------------------------------------------------------------------
bool ivec2::operator==(const ivec2& compare) const {
	return x == compare.x && y == compare.y;
}


//-----------------------------------------------------------------------------------------------
bool ivec2::operator!=(const ivec2& compare) const {
	return x != compare.x || y != compare.y;
}

void ivec2::fromString(const char* data) {
  auto raw = split(data, " ,");
  GUARANTEE_OR_DIE(raw.size() == 2, "illegal input string to parse");

  x = parse<int>(raw[0]);
  y = parse<int>(raw[1]);
}

std::string ivec2::toString() const {
  return Stringf("%d,%d", x, y);
}

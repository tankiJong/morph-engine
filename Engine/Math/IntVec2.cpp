#include "IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
const IntVec2 IntVec2::zero = IntVec2(0, 0);
const IntVec2 IntVec2::top = IntVec2(0, 1);
const IntVec2 IntVec2::down = IntVec2(0, -1);
const IntVec2 IntVec2::left = IntVec2(-1, 0);
const IntVec2 IntVec2::right = IntVec2(1, 0);

//-----------------------------------------------------------------------------------------------
IntVec2::IntVec2(const IntVec2& copy)
	: x(copy.x)
	, y(copy.y) {}


//-----------------------------------------------------------------------------------------------
IntVec2::IntVec2(int initialX, int initialY)
	: x(initialX)
	, y(initialY) {}

IntVec2::IntVec2(const vec2& castFrom)
  : x((int)castFrom.x)
  , y((int)castFrom.y) {
}

IntVec2::IntVec2(std::initializer_list<int> list) 
  : x(*list.begin())
  , y(*(list.begin() + 1)) {
}

//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator + (const IntVec2& vecToAdd) const {
	return IntVec2(this->x + vecToAdd.x, this->y + vecToAdd.y);
}

const vec2 IntVec2::operator+(const vec2& vecToAdd) const {
  return vecToAdd + *this;
}

//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator-(const IntVec2& vecToSubtract) const {
	return IntVec2(this->x - vecToSubtract.x, this->y - vecToSubtract.y);
}


//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator*(int uniformScale) const {
	return IntVec2(this->x*uniformScale, this->y*uniformScale);
}


int IntVec2::operator*(const IntVec2& another) const {
	return x*another.x + y*another.y;
}

//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator/(int inverseScale) const {
	return IntVec2(this->x / inverseScale, this->y / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator+=(const IntVec2& vecToAdd) {
	x = x + vecToAdd.x;
	y = y + vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator-=(const IntVec2& vecToSubtract) {
	x = x - vecToSubtract.x;
	y = y - vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator*=(const int uniformScale) {
	x = x * uniformScale;
	y = y * uniformScale;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator/=(const int uniformDivisor) {
	x = x / uniformDivisor;
	y = y / uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator=(const IntVec2& copyFrom) {
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const IntVec2 operator*(int uniformScale, const IntVec2& vecToScale) {
	return IntVec2(vecToScale.x * uniformScale, vecToScale.y * uniformScale);
}

int dotProduct(const IntVec2 & a, const IntVec2 & b) {
	return a*b;
}


//-----------------------------------------------------------------------------------------------
bool IntVec2::operator==(const IntVec2& compare) const {
	return x == compare.x && y == compare.y;
}


//-----------------------------------------------------------------------------------------------
bool IntVec2::operator!=(const IntVec2& compare) const {
	return x != compare.x || y != compare.y;
}

void IntVec2::fromString(const char* data) {
  auto raw = split(data, " ,");
  GUARANTEE_OR_DIE(raw.size() == 2, "illegal input string to parse");

  x = parse<int>(raw[0]);
  y = parse<int>(raw[1]);
}

std::string IntVec2::toString() const {
  return Stringf("%d,%d", x, y);
}

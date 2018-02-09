#include "IntRange.hpp"
#include "MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

IntRange::IntRange()
  : min(0)
  , max(0) {
    
}

IntRange::IntRange(int minInclusive, int maxInclusive)
: min(minInclusive)
, max(maxInclusive){}
IntRange::IntRange(int minMax)
: min(minMax)
, max(minMax)
{}
bool IntRange::isOverlappedWith(const IntRange& another) {
  return areRangesOverlap(*this, another);
}

bool IntRange::isInRange(int number) const {
  return number >= min && number <= max;
}

int IntRange::getRandomInRange() const {
  return getRandomInt32(min, max);
}

int IntRange::numIntIncluded() const {
  return max - min + 1;
}

int IntRange::size() const {
  return max - min;
}

void IntRange::fromString(const char* data) {
  auto raw = split(data, "~");
  GUARANTEE_OR_DIE(raw.size() == 2 || raw.size() == 1, "illegal input string to parse");

  min = parse<int>(raw[0]);
  max = parse<int>(raw[raw.size() - 1]);
}

void IntRange::reset() {
  min = 0;
  max = 0;
}

std::string IntRange::toString() const {
  return Stringf("%d~%d", min, max);
}

bool areRangesOverlap(const IntRange& a, const IntRange& b) {
  return !(b.min > a.max || a.min > b.max);
}

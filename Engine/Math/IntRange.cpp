#include "IntRange.hpp"
#include "MathUtils.hpp"

IntRange::IntRange(int minInclusive, int maxInclusive)
: min(minInclusive)
, max(maxInclusive){}
IntRange::IntRange(int minMax)
: min(minMax)
, max(minMax)
{}
bool IntRange::isOverlappedWith(const IntRange& another) {
  return areIntRangesOverlap(*this, another);
}
int IntRange::getRandomInRange() const {
  return getRandomInt32(min, max);
}

int IntRange::numIntIncluded() const {
  return max - min + 1;
}

bool areIntRangesOverlap(const IntRange& a, const IntRange& b) {
  return !(b.min > a.max || a.min > b.max);
}

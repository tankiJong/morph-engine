#include "FloatRange.hpp"

#include "MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

FloatRange::FloatRange(float minInclusive, float maxInclusive)
  : min(minInclusive)
  , max(maxInclusive) {}
FloatRange::FloatRange(float minMax)
  : min(minMax)
  , max(minMax) {}
bool FloatRange::isOverlappedWith(const FloatRange& another) {
  return areRangesOverlap(*this, another);
}
float FloatRange::getRandomInRange() const {
  return getRandomf(min, max);
}

void FloatRange::fromString(const char* data) {
  auto raw = split(data, " ,");
  GUARANTEE_OR_DIE(raw.size() == 2 || raw.size() == 1, "illegal input string to parse");

  min = parse<float>(raw[0]);
  max = parse<float>(raw[raw.size() - 1]);
}

std::string FloatRange::toString() const {
  return Stringf("%f~%f", min, max);
}

bool areRangesOverlap(const FloatRange& a, const FloatRange& b) {
  return !(b.min > a.max || a.min > b.max);
}

#include "FloatRange.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

FloatRange::FloatRange(float minInclusive, float maxInclusive)
  : min(minInclusive)
  , max(maxInclusive) {}
FloatRange::FloatRange(float minMax)
  : min(minMax)
  , max(minMax) {}
bool FloatRange::isOverlappedWith(const FloatRange& another) {
  return areRangesOverlap(*this, another);
}

bool FloatRange::isInRange(float number) const {
  return number >= min && number <= max;
}

float FloatRange::getRandomInRange() const {
  return getRandomf(min, max);
}

void FloatRange::fromString(const char* data) {
  auto raw = split(data, "~ ");
  GUARANTEE_OR_DIE(raw.size() == 2 || raw.size() == 1, "illegal input string to parse");

  float a = parse<float>(raw[0]);
  float b = parse<float>(raw[raw.size() - 1]);

  max = std::max(a, b);
  min = std::min(a, b);
}

std::string FloatRange::toString() const {
  return Stringf("%f~%f", min, max);
}

bool areRangesOverlap(const FloatRange& a, const FloatRange& b) {
  return !(b.min > a.max || a.min > b.max);
}

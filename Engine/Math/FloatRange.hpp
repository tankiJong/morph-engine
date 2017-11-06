#pragma once
#include <string>

class FloatRange {
public:
  float max;
  float min;
  explicit FloatRange(float minInclusive, float maxInclusive);
  explicit FloatRange(float minMax);
  bool isOverlappedWith(const FloatRange& another);
  float getRandomInRange() const;

  void fromString(const char* data);
  std::string toString() const;
};
bool areRangesOverlap(const FloatRange& a, const FloatRange& b);

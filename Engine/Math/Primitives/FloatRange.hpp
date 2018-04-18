#pragma once
#include <string>

class FloatRange {
public:
  float min = 0;
  float max = 0;
  FloatRange() {}
  FloatRange(float minInclusive, float maxInclusive);
  explicit FloatRange(float minMax);
  bool isOverlappedWith(const FloatRange& another);
  bool isInRange(float number) const;
  float getRandomInRange() const;

  void fromString(const char* data);
  float length() const;
  std::string toString() const;
};
bool areRangesOverlap(const FloatRange& a, const FloatRange& b);

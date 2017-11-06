#pragma once
#include <string>

class IntRange {
public:
  int max;
  int min;
  IntRange();
  explicit IntRange(int minInclusive, int maxInclusive);
  explicit IntRange(int minMax);
  bool isOverlappedWith(const IntRange& another);
  int getRandomInRange() const;
  int numIntIncluded() const;

  void fromString(const char* data);
  std::string toString() const;
};
bool areRangesOverlap(const IntRange& a, const IntRange& b);

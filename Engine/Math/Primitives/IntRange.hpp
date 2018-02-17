#pragma once
#include <string>

class IntRange {
public:
  int max;
  int min;
  IntRange();
  IntRange(int minInclusive, int maxInclusive);
  explicit IntRange(int minMax);
  bool isOverlappedWith(const IntRange& another);
  bool isInRange(int number) const;
  int getRandomInRange() const;
  int numIntIncluded() const;
  int size() const;
  void fromString(const char* data);
  void reset();
  std::string toString() const;
};
bool areRangesOverlap(const IntRange& a, const IntRange& b);

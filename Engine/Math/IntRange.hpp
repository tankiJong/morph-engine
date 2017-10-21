#pragma once

class IntRange {
public:
  int max;
  int min;
  explicit IntRange(int minInclusive, int maxInclusive);
  explicit IntRange(int minMax);
  bool isOverlappedWith(const IntRange& another);
  int getRandomInRange() const;
  int numIntIncluded() const;
};
bool areIntRangesOverlap(const IntRange& a, const IntRange& b);

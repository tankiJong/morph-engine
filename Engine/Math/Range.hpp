#pragma once
#include <string>
#include "Engine/Core/common.hpp"
#include <optional>
#include <stdlib.h>

//
// #pragma once
// #include <string>
//
// class IntRange {
// public:
//   int max;
//   int min;
//   IntRange();
//   IntRange(int minInclusive, int maxInclusive);
//   explicit IntRange(int minMax);
//   bool isOverlappedWith(const IntRange& another);
//   bool isInRange(int number) const;
//   int getRandomInRange() const;
//   int numIntIncluded() const;
//   int size() const;
//   void fromString(const char* data);
//   void reset();
//   std::string toString() const;
// };
// bool areRangesOverlap(const IntRange& a, const IntRange& b);


template<typename T>
class Range {
  static_assert(std::is_default_constructible_v<T>);
  static_assert(std::is_trivial_v<T>);
  static_assert(std::is_same_v<std::decay_t<T>, T>);
public:
  T mins, maxs;

  Range() = default;
  Range(const T& minInclusive, const T& maxInclusive)
    : mins(minInclusive)
    , maxs(maxInclusive) {}
  explicit Range(const T& v): mins(v), maxs(v) {}

  inline bool empty() { return mins > maxs; }
  inline bool isOverlap(const Range<T>& rhs) {
    return !(rhs.mins > maxs || mins > rhs.maxs);
  }

  inline bool contains(const T& value) { return value >= mins && value <= maxs; }

  inline Range<T> intersection(const Range<T>& rhs) const {
    return { std::max(mins, rhs.mins), std::min(maxs, rhs.maxs) };
  }
  inline T size() const { return maxs - mins; }

  void grow(const T& x) {
    mins = std::min(x, mins);
    maxs = std::max(x, maxs);
  }
};
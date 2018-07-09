#pragma once
#include <string>
#include "Engine/Core/common.hpp"
#include <optional>

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
  T min, max;

  Range() = default;
  Range(const T& minInclusive, const T& maxInclusive)
    : min(minInclusive)
    , max(maxInclusive) {}
  explicit Range(const T& v): min(v), max(v) {}

  inline bool empty() { return min > max; }
  inline bool isOverlap(const Range<T>& rhs) {
    return !(rhs.min > max || min > rhs.max);
  }

  inline bool contains(const T& value) { return value >= min && value <= max; }

  inline Range<T> intersection(const Range<T>& rhs) const {
    return { std::max(min, rhs.min), std::min(max, rhs.max) };
  }
  inline T size() const { return max - min; }

  void grow(const T& x) {
    min = std::min(x, min);
    max = std::max(x, max);
  }
};
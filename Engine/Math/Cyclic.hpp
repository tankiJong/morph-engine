#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

template<typename UintType>
bool cycLess(UintType a, UintType b) {
  constexpr UintType HALF_MAX = 0x1 << (8 * sizeof(UintType) - 1);
  
  UintType diff = b - a;
  return diff > 0 && (diff <= HALF_MAX - 1);
}

template<typename UintType>
bool cycLessEq(UintType a, UintType b) {
  constexpr UintType HALF_MAX = 0x1 << (8 * sizeof(UintType) - 1);

  UintType diff = b - a;
  return diff <= HALF_MAX - 1;
}

template<typename UintType>
bool cycGreater(UintType a, UintType b) {
  return !cycLessEq(a, b);
}

bool cycLess(float a, float b, float range) {
  float diff = b - a;

  float mdiff = fmodf(diff, range);
  if(mdiff < 0) {
    mdiff += range;
  }

  return mdiff <= (range * .5f);
}
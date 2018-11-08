//-----------------------------------------------------------------------------------------------
// Time.hpp
//
#pragma once
#include "Engine/Core/common.hpp"
#include <stdint.h>

//-----------------------------------------------------------------------------------------------

struct Time {
  Time();

  Time& operator+=(const Time& rhs);
  double second;
  uint64_t hpc;
  uint millisecond;

};

double GetCurrentTimeSeconds();

uint64_t __fastcall GetPerformanceCounter();

double PerformanceCountToSecond(uint64_t count);

std::string beautifySeconds(double seconds);

struct Timestamp {
  int64 stamp;
  Timestamp();
  std::string toString() const;

  static Timestamp ms(float millisecond);
};
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

class ProfileLogScope {
public:
  ProfileLogScope(const char* tag);
  ~ProfileLogScope();
public:
  const char* mTag;
  uint64_t mStartHPC;
};
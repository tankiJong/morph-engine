//-----------------------------------------------------------------------------------------------
// Time.cpp
//	

//-----------------------------------------------------------------------------------------------
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Core/StringUtils.hpp"
#include <time.h>

Time::Time() {
  memset(this, 0, sizeof(Time));
}

Time& Time::operator+=(const Time& rhs) {
  second += rhs.second;
  hpc += rhs.hpc;
  millisecond += rhs.millisecond;
  return *this;
}


struct TimeSystem {
  TimeSystem() {
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    mFrequency = *(uint64_t*)&li;
    mSecondsPerCount = 1.0 / (double)mFrequency;
  }

public:
  uint64_t mFrequency;
  double mSecondsPerCount;
};


static TimeSystem g_timeSystem;
//-----------------------------------------------------------------------------------------------
double InitializeTime( LARGE_INTEGER& out_initialTime )
{
	LARGE_INTEGER countsPerSecond;
	QueryPerformanceFrequency( &countsPerSecond );
	QueryPerformanceCounter( &out_initialTime );
	return( 1.0 / static_cast< double >( countsPerSecond.QuadPart ) );
}

//-----------------------------------------------------------------------------------------------
double GetCurrentTimeSeconds()
{
	static LARGE_INTEGER initialTime;
	static double secondsPerCount = InitializeTime( initialTime );
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter( &currentCount );
	LONGLONG elapsedCountsSinceInitialTime = currentCount.QuadPart - initialTime.QuadPart;

	double currentSeconds = static_cast< double >( elapsedCountsSinceInitialTime ) * secondsPerCount;
	return currentSeconds;
} 

uint64_t __fastcall GetPerformanceCounter() {
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);

  return *(uint64_t*)&li;
}

double PerformanceCountToSecond(uint64_t count) {
  return (double)count * g_timeSystem.mSecondsPerCount;
}

std::string beautifySeconds(double seconds) {
  double absSec = abs(seconds);
  if(absSec < 1e-3) {
    return Stringf("%.3lf us", seconds * 1.0e6);
  }

  if(absSec < 1e-1) {
    return Stringf("%.3lf ms", seconds * 1.0e3);
  }

  return Stringf("%.3lf s", seconds);
}

Timestamp::Timestamp() {
  time_t t;
  ::time(&t);
  stamp = (int64)t;
}

std::string Timestamp::toString() const {
  tm t;
  localtime_s(&t, &stamp);

  return Stringf("%i-%i-%i-%i-%i-%i",   t.tm_year + 1900,
                                        t.tm_mon + 1,
                                        t.tm_mday,
                                        t.tm_hour,
                                        t.tm_min,
                                        t.tm_sec);
}

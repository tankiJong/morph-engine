#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/Time/Time.hpp"
#include <vector>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Game/Gameplay/EncounterState.hpp"

class Clock {
public:
  friend class Stopwatch;
  
  /**
   * \brief create a master clock
   */
  Clock();
  Clock(Clock&) = delete;

  void reset();
  void beginFrame();
  void stepClock(uint64_t elapsed);
  Clock* createChild();
  inline void pause() { paused = !paused; }
  inline void scale(float s = 1.f) { EXPECTS(s > 0);  mScale = s; }
public:
  Time frame;
  Time total;
  bool paused = false;
  std::vector<Clock*> mChildren;


protected:
  Clock(Clock* parent);
  void addChild(Clock* clock);
  float mScale = 1.f;
  Clock* mParent = nullptr;
  uint frameCount = 0u;
  uint64_t mLastFrameHPC = 0u;
};


class Stopwatch {
  friend class Clock;
  friend Stopwatch& createWatch();
public:
  inline void pause() { isPaused = true; };
  inline void resume() { isPaused = false; }
  Stopwatch();
protected:
  void elapse();
  Time currentTime;
  Time startTime;
  bool isPaused = false;
};

// add delegate callback for countdown
class Countdown: public Stopwatch {
  friend class CdHeap;
  friend Countdown& createCountdown(double duration);
public:
  double duration = 0;
  inline bool isFinished() const { return currentTime.second - startTime.second >= duration; }
  Countdown(double dura);
};


Clock& GetMainClock();

Stopwatch& createWatch();
bool destoryWatch(Stopwatch& sw);

Countdown& createCountdown(double duration);
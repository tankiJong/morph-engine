#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/Time/Time.hpp"
#include <vector>


class Clock {
public:

  
  /**
   * \brief create a master clock
   */
  Clock();
  Clock(Clock&) = delete;

  void reset();
  void beginFrame();
  void stepClock(uint64_t elapsed);
  void createChild();
public:
  Time frame;
  Time total;
  std::vector<Clock*> mChildren;


protected:
  Clock(Clock* parent = nullptr);
  void addChild(Clock* clock);
  float scale = 1.f;
  bool isPaused = false;
  Clock* mParent = nullptr;
  uint frameCount;
  uint64_t mLastFrameHPC;
};


class Stopwatch {
  friend class Clock;
  friend Stopwatch& createWatch();

public:
  inline void pause() { isPaused = true; };
  inline void resume() { isPaused = false; }
protected:
  Stopwatch();
  void elapse();
  Time currentTime;
  Time startTime;
  bool isPaused;
};

class Countdown: public Stopwatch {
  friend class CountdownHeap;
  friend Countdown& createCountdown(double duration);
public:
  float duration = 0;
  inline bool isFinished() const { return currentTime.second - startTime.second >= duration; }
  Countdown(double dura);
};


Clock& GetMainClock();

Stopwatch& createWatch();
bool destoryWatch(Stopwatch& sw);

Countdown& createCountdown(double duration);
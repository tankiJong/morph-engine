#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/Time/Time.hpp"
#include <vector>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Game/GameCommon.hpp"

class Clock {
public:
  friend class Interval;
  
  /**
   * \brief create a master clock
   */
  Clock();
  Clock(Clock&) = delete;

  void reset();
  void beginFrame();
  void stepClock(uint64_t elapsed);
  owner<Clock*> createChild();
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


class Interval {
  friend class Clock;
  friend Interval& createWatch();
public:
  Interval(double duration);
  Interval();
  ~Interval();
  inline void pause() { isPaused = true; };
  inline void resume() { isPaused = false; }
  
  bool decrement();

  double duration;
  uint flush();
  Clock& clock() const;
protected:
  double currentTime() const;
  double mStartTime;
  bool isPaused = false;
};

Clock& GetMainClock();

Interval& createWatch();

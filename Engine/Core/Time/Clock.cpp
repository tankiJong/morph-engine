#include "Clock.hpp"
#include <queue>
#include <list>

static Clock gMainClock;

std::list<Interval> gStopwatch;

owner<Clock*> Clock::createChild() {
  return new Clock(this);
}

Clock::Clock(Clock* parent) {
  reset();

  if (parent != nullptr) {
    parent->addChild(this);
  }
}

Clock::Clock() {
  reset();
}

void Clock::reset() {
  mLastFrameHPC = GetPerformanceCounter();
  memset(&frame, 0, sizeof(frame));
  memset(&total, 0, sizeof(total));
}

void Clock::beginFrame() {
  uint64_t current = GetPerformanceCounter();
  uint64_t elapsed = current - mLastFrameHPC;
  
  stepClock(elapsed);

  mLastFrameHPC = current;
}

void Clock::stepClock(uint64_t elapsed) {
  frameCount++;
  if (paused) {
    elapsed = 0;
  } else {
    elapsed = (uint64_t)((double)elapsed*mScale);
  }

  if(this == &gMainClock) {
    for(Interval& sw: gStopwatch) {
      sw.elapse();
    }
  }

  double elapsedSec = PerformanceCountToSecond(elapsed);

  frame.second = elapsedSec;
  frame.hpc = elapsed;
  frame.millisecond = (uint)frame.second * 1000;

  total += frame;

  for(Clock* c: mChildren) {
    c->stepClock(elapsed);
  }
}

void Clock::addChild(Clock* clock) {
  mChildren.push_back(clock);
  clock->mParent = this;
}

uint Interval::flush() {
  uint d = 0;
  while(decrement()) {
    d++;
  }
  return d;
}

Clock& Interval::clock() const {
  return gMainClock;
}

Interval::Interval() {
  mStartTime = gMainClock.total.second;
  mCurrentTime = mStartTime;
}

void Interval::elapse() {
  mCurrentTime = gMainClock.total.second;
}

Clock& GetMainClock() {
  return gMainClock;
}

Interval& createWatch() {
  return gStopwatch.emplace_back();
}

bool destoryWatch(Interval& target) {
  for(auto it = gStopwatch.begin(); it != gStopwatch.end(); it++) {
    if(&(*it) == &target) {
      gStopwatch.erase(it);
      return true;
    }
  }
  return false;
}


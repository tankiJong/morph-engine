﻿#include "Clock.hpp"
#include <queue>
#include <list>

static Clock gMainClock;

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

bool Interval::decrement() {
  if (currentTime() - mStartTime > duration) {
    mStartTime += duration;
    return true;
  }
  return false;
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

double Interval::currentTime() const {
  return clock().total.second;
}

Interval::Interval(double dura)
  : Interval() {
  duration = dura;
}

Interval::Interval() {
  mStartTime = gMainClock.total.second;
}

Interval::~Interval() {
}

Clock& GetMainClock() {
  return gMainClock;
}



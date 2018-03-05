#include "Clock.hpp"
#include <queue>
#include <list>

static Clock gMainClock;

static struct CountdownComp {// greator<Countdown>
  bool operator()(const Countdown& a, const Countdown& b) {
    return a.duration > b.duration;
  }
} SwComp;

class CdHeap: protected std::vector<Countdown> {
public:
  using ContainerType = std::vector<Countdown>;
  CdHeap() {}

  auto begin() { return ContainerType::begin(); }
  auto end() { return ContainerType::end(); }

  template<typename ...Valty>
  Countdown& emplace_push(Valty ...valty) {
    Countdown& cd = emplace_back(valty...);
    std::push_heap(begin(), end(), SwComp);
    return cd;
  }

  Countdown& push(const Countdown& sw) {
    push_back(sw);
    std::push_heap(begin(), end(), SwComp);
    return back();
  }

  Countdown& top() {
    return back();
  }

  void pop() {
    std::pop_heap(begin(), end(), SwComp);
    pop_back();
  }

  uint size() {
    return  ContainerType::size();
  }
};

static CdHeap gCountdown;
std::list<Stopwatch> gStopwatch;

void Clock::createChild() {
  new Clock(this);
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
}

void Clock::stepClock(uint64_t elapsed) {
  frameCount++;
  if (isPaused) {
    elapsed = 0;
  } else {
    elapsed = (uint64_t)((double)elapsed*scale);
  }

  if(this == &gMainClock) {
    for(Countdown& cd: gCountdown) {
      cd.elapse();
    }

    for(Stopwatch& sw: gStopwatch) {
      sw.elapse();
    }

    while(gCountdown.size() > 0 && gCountdown.top().isFinished()) {
      gCountdown.pop();
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

Stopwatch::Stopwatch() {
  startTime = gMainClock.total;
  currentTime = startTime;
}

void Stopwatch::elapse() {
  currentTime = gMainClock.total;
}

Countdown::Countdown(double dura) {
  duration = dura;
}

Clock& GetMainClock() {
  return gMainClock;
}

Stopwatch& createWatch() {
  return gStopwatch.emplace_back();
}

bool destoryWatch(Stopwatch& target) {
  for(auto it = gStopwatch.begin(); it != gStopwatch.end(); it++) {
    if(&(*it) == &target) {
      gStopwatch.erase(it);
      return true;
    }
  }
  return false;
}

Countdown& createCountdown(double duration) {
  return gCountdown.emplace_push(duration);
}


#include "Profiler.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include <stack>
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Debug/Profile/Overlay.hpp"
#include "Engine/Debug/Console/Command.hpp"

using namespace Profile;

bool gPause = false;

namespace Profile {

  class Profiler {
  public:
    void push(const char* id);
    void pop();
    void begin();

    prof_sample_t* query(uint frameOffsetFromCurrent) {
      EXPECTS(frameOffsetFromCurrent <= MAX_FRAME_RECORDED);
      return (frameOffsetFromCurrent > prevStack.size()) 
        ? nullptr : prevStack[prevStack.size() - frameOffsetFromCurrent];
    }

    bool toggle(bool toPause) { toBePaused = toPause; return toBePaused; }
  protected:
    static void destorySampleTreeRecursive(prof_sample_t* prevStack);

    struct SampleBuffer {
      operator prof_sample_t*() const {
        return buffer[index];
      };

      void operator=(prof_sample_t* rhs) {
        index = (index+1) % MAX_FRAME_RECORDED;
        if (buffer[index]) {
          destorySampleTreeRecursive(buffer[index]);
        }
        buffer[index] = rhs;
      }

      prof_sample_t* operator[](uint idx) const {
        EXPECTS(index <= size());
        uint vIndex = size() < MAX_FRAME_RECORDED ? idx : (index + 1 + idx) % MAX_FRAME_RECORDED;
        return buffer[vIndex];
      }

      uint size() const {
        return buffer[(index + 1) % MAX_FRAME_RECORDED] == nullptr ? index : MAX_FRAME_RECORDED;
      }

      ~SampleBuffer() {
        for (prof_sample_t* sample : buffer) {
          destorySampleTreeRecursive(sample);
        }
      }
    protected:
      uint index = MAX_FRAME_RECORDED - 1;
      prof_sample_t* buffer[MAX_FRAME_RECORDED] = { nullptr };
    };

    prof_sample_t* stack = nullptr;
    SampleBuffer prevStack;
    bool toBePaused = false;
    bool isPaused = false;
  };
}



void prof_sample_t::addChild(prof_sample_t* ps) {
  if(mChildren == nullptr) {
    mChildren = ps;
    return;
  }
  mChildren->mNext = ps;
  ps->mPrev = mChildren;
  mChildren = ps;
}

void prof_sample_t::finish() {
  endHpc = GetPerformanceCounter();
}

double prof_sample_t::elapsedTime() const {
  return PerformanceCountToSecond(endHpc - starHpc);
}

owner<prof_sample_t*> prof_sample_t::create(const char* id) {
  prof_sample_t* sample = new prof_sample_t;

  strcpy_s(sample->id, id);
  sample->starHpc = GetPerformanceCounter();

  return sample;
}

Report& prof_sample_t::dump(Report::eViewOption view) {
  if (!mReport) {
    mReport = Report();
    mReport->fromSample(this, view);
  }

  return mReport.value();
}

Profiler* gProfiler;

void Profile::startup() {
#ifdef PROFILER_ENABLED
  gProfiler = new Profiler();
  initOverlay();
#endif
}

prof_sample_t* Profile::dump(uint frameOffsetFromCurrent) {
#ifdef PROFILER_ENABLED
  EXPECTS(frameOffsetFromCurrent <= MAX_FRAME_RECORDED);

  return gProfiler->query(frameOffsetFromCurrent);
#else
  return nullptr;
#endif
}

void Profile::push(const char* id) {
#ifdef PROFILER_ENABLED
  gProfiler->push(id);
#endif
}

void Profile::pop() {
#ifdef PROFILER_ENABLED
  gProfiler->pop();
#endif
}

void Profile::markFrame() {
#ifdef PROFILER_ENABLED
  gProfiler->toggle(gPause);
  gProfiler->begin();
#endif
}

void Profile::pause() {
  gPause = true;
}

void Profile::resume() {
  gPause = false;
}

void Profiler::begin() {

  if (stack != nullptr) {
    // ...
    prevStack = stack;

    pop();

    ENSURES(stack == nullptr);
  }

  if (isPaused != toBePaused) {
    isPaused = toBePaused;
    if (isPaused) return;
  }

  push("__Frame__");
}

void Profiler::pop() {
  if (isPaused) return;
  ENSURES(stack != nullptr);
  stack->finish();
  stack = stack->mParent;
}

void Profiler::destorySampleTreeRecursive(prof_sample_t* prevStack) {
  if (prevStack == nullptr) return;
  std::stack<prof_sample_t*> pending;

  pending.push(prevStack);

  while(!pending.empty()) {
    prof_sample_t* top = pending.top();
    if(top->mChildren) {
      prof_sample_t* head = top->mChildren;
      while(head != nullptr) {
        pending.push(head);
        head = head->mPrev;
      }
      top->mChildren = nullptr;
      continue;
    } else {
      delete top;
      pending.pop();
    }
  }
}

void Profiler::push(const char* id) {
  if (isPaused) return;
  prof_sample_t* measure = prof_sample_t::create(id);
  if (stack == nullptr) {
    stack = measure;
  } else {
    measure->mParent = stack;
    stack->addChild(measure);
    stack = measure;
  }
}


template<typename T>
class PageAllocator {
  struct llp_t {
    // llp_t* prev;
    llp_t* next;
  };

public:
  T* allocate() {
    llp_t* head = mFreeList;
    if(head == nullptr) {
      allocateBlock();
      head = mFreeList;
    } else {
      // LLP_REMOVE(mFreeList, head);
      mFreeList = mFreeList->next;
    }

    T* obj = new (head) T();
    return obj;
  }

  void destroy(T* obj) {
    obj->~T();
    llp_t* iter = (llp_t*)obj;

    LLP_APPEND(mFreeList, iter);
    iter->next = mFreeList;
    mFreeList->next = iter;
  }

  llp_t* createBlock() {
    return (llp_t*)malloc(sizeof(T));
  }
protected:
  llp_t* mFreeList;
};

COMMAND_REG("profiler_pause", "", "pause the profiler")(Command& cmd) {
  Profile::pause();
  return true;
}

COMMAND_REG("profiler_resume", "", "resume the profiler")(Command& cmd) {
  Profile::resume();
  return true;
}

COMMAND_REG("profiler_report", "eViewOption: flat|tree", "print the last frame report")(Command& cmd) {
  if (cmd.arg<0>() == "flat") {
    dump(1)->dump(Report::VIEW_FLAT).log(Report::VIEW_FLAT);
    return true;
  }
  if (cmd.arg<0>() == "tree") {
    dump(1)->dump(Report::VIEW_TREE).log(Report::VIEW_TREE);
    return true;
  }
  throw InvalidArgumentException(0);

  return false;
}
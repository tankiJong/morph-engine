#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Debug/Profile/Report.hpp"
#include <optional>
#include "Engine/Debug/Draw.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Config.hpp"
#include "Engine/Core/Time/Time.hpp"

namespace Profile {
  struct prof_sample_t {
    char id[64] = "Invalid";
    u64 starHpc = uint(-1);
    u64 endHpc = uint(-1);

    void addChild(prof_sample_t* ps);
    void finish();
    double elapsedTime() const;

    inline const prof_sample_t* children() const { return mChildren; }
    inline const prof_sample_t* parent() const { return mParent; }
    inline const prof_sample_t* prev() const { return mPrev; }
    inline const prof_sample_t* next() const { return mNext; }

    inline prof_sample_t* childHead() const { return mChildren; };
    static owner<prof_sample_t*> create(const char* id);

    Report& dump(Report::eViewOption view = Report::VIEW_TREE);
  protected:
    friend class Profiler;
    prof_sample_t() = default;
    prof_sample_t* mChildren = nullptr;
    prof_sample_t* mParent = nullptr;
    prof_sample_t  *mPrev = nullptr, *mNext = nullptr; // sublings
    std::optional<Report> mReport = std::nullopt;
  };

  static constexpr uint MAX_FRAME_RECORDED = 1024u;

  void startup();
  prof_sample_t* dump(uint frameOffsetFromCurrent = 1);

  void push(const char* id);
  void pop();

  void markFrame();

  void pause();
  void resume();

  template<bool LOG>
  class Scoped {
  public:
    Scoped(const char* id): id(id) {
      push(id);
      if constexpr (LOG) {
        startHps = GetPerformanceCounter();
      }
    }
    ~Scoped() {
      pop();
      if constexpr (LOG) {
        u64 endHps = GetPerformanceCounter();
        double time = PerformanceCountToSecond(endHps - startHps);

        Debug::log(Stringf("[%s]%lf seconds", id, time));
      }
      
    }

    u64 startHps;
    const char* id;
  };

} // Profile



#ifdef PROFILER_ENABLED
#define PROF_SCOPE(tag) Profile::Scoped<false> APPEND(__Scoped_, __LINE__)(tag);
#define PROF_SCOPE_LOG(tag) Profile::Scoped<true> APPEND(__Log_Scoped_, __LINE__)(tag);
#else
#define PROF_SCOPE(tag) ;
#endif

#define PROF_FUNC() PROF_SCOPE(__FUNCTION__)
#define PROF_FUNC_LOG() PROF_SCOPE_LOG(__FUNCTION__)
#pragma once
#include "Engine/Core/common.hpp"
#include <vector>
#include <map>

namespace Profile {
  struct prof_sample_t;

  class Report {
  public:
    enum eViewOption {
      VIEW_FLAT,
      VIEW_TREE,
    };

    enum eSortOption {
      SORT_UNKNOWN,
      SORT_TOTAL_TIME,
      SORT_SELF_TIME,
    };
    struct Entry {
      friend class Report;
      std::string_view name;
      uint callCount;
      double totalTime; // inclusive time
      double selfTime;  // exclusive time
      double childTime;
      double totalTimeAveragePerCall;
      double selfTimeAveragePerCall;

      void populateTree(const prof_sample_t& node);
      void populateFlat(const prof_sample_t& node);
      Entry* aquireChild(const char* id);
      inline const std::vector<std::pair<std::string, Entry*>>& children() const { return mChildren; }
      Entry* parent() const { return mParent; }

      void clear();
      void sort(eSortOption option);
      ~Entry();
    protected:
      void accumulate(const prof_sample_t& node);
      Entry* mParent = nullptr;
      std::vector<std::pair<std::string, Entry*>> mChildren;
      eSortOption sorting = SORT_UNKNOWN;
    };
    void fromSample(prof_sample_t* sample, eViewOption view);
    void accumlateSample(prof_sample_t* sample, eViewOption view);
    void sort(eSortOption op) { root.sort(op); }
    void computeSelfTime();

    void log(eViewOption option) const;
    double totalFrameTime() const;
    inline const Entry& self() const { return root; }
  protected:
    Entry root;
  };
  
}

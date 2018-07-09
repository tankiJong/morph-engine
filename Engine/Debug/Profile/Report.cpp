#include "Report.hpp"
#include "Engine/Debug/Profile/Profiler.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include <stack>
#include "Engine/Renderer/Font.hpp"

using namespace Profile;

void Report::Entry::populateTree(const prof_sample_t& node) {

  accumulate(node);

  const prof_sample_t* head = node.children();

  while(head != nullptr) {
    Entry* entry = aquireChild(head->id);
    entry->populateTree(*head);
    // EXPECTS(childTime <= totalTime);
    head = head->prev();
  }

}

void Report::Entry::populateFlat(const prof_sample_t& node) {

  const prof_sample_t* head = node.children();

  while(head != nullptr) {
    Entry* entry = aquireChild(head->id);
    entry->accumulate(*head);
    // EXPECTS(parent->totalTime >= parent->childTime);
    populateFlat(*head);
    head = head->prev();
  }
}

Report::Entry* Report::Entry::aquireChild(const char* id) {
  for(auto& [k, v]: mChildren) {
    if(k == id) {
      return v;
    }
  }

  Entry* e = new Entry();
  e->mParent = this;
  auto& node = mChildren.emplace_back();
  node.first = std::string(id);
  node.second = e;
  e->name = id;
  return e;
  
}

void Report::Entry::clear() {
  callCount = 0;
  totalTime = 0;
  selfTime = 0;

  for(auto&& [k, v]: mChildren) {
    v->clear();
  }
}

void Report::Entry::sort(eSortOption option) {
  if (sorting == option) return;

  using eleType = std::pair<std::string, Entry*>;
  switch(option) {
    case SORT_TOTAL_TIME:
      std::sort(mChildren.begin(), mChildren.end(), [](eleType& a, eleType& b) {
        return a.second->totalTime < b.second->totalTime;
      });
    break;
    case SORT_SELF_TIME:
      std::sort(mChildren.begin(), mChildren.end(), [](eleType& a, eleType& b) {
        return a.second->selfTime < b.second->selfTime;
      });
    break;
    default:
      ERROR_AND_DIE("INVALID sorting option");
    ;
  }

  for(auto [_, v]: mChildren) {
    v->sort(option);
  }

  sorting = option;
}

Report::Entry::~Entry() {
  for(auto child: mChildren) {
    SAFE_DELETE(child.second);
  }
}

void Report::computeSelfTime() {
  std::stack<Entry*> entries;

  entries.push(&root);

  while(!entries.empty()) {
    Entry* entry = entries.top();
    entries.pop();

    double childTime = 0;
    for(auto& child: entry->mChildren) {
      entries.push(child.second);
      childTime += child.second->totalTime;
    }
    entry->childTime = childTime;
    entry->selfTime = entry->totalTime - entry->selfTime;
    entry->selfTimeAveragePerCall = entry->selfTime / (double)entry->callCount;
  }
}

void Report::Entry::accumulate(const prof_sample_t& node) {
  // handle self
  callCount++;
  totalTime += node.elapsedTime();
  totalTimeAveragePerCall = totalTime / (float)callCount; 
}

void Report::fromSample(prof_sample_t* sample, eViewOption view) {
  root.clear();
  accumlateSample(sample, view);
}

void Report::accumlateSample(prof_sample_t* sample, eViewOption view) {
  root.name = sample->id;
  switch (view) {
    case VIEW_FLAT:
      root.accumulate(*sample);
      root.populateFlat(*sample);
      break;
    case VIEW_TREE:
      root.populateTree(*sample);
      break;
    default:;
  }
  computeSelfTime();
}

void Report::log(eViewOption option) const {
  struct Iter {
    const Entry* entry = nullptr;
    uint depth = 0;
  };

  std::stack<Iter> toProcess;

  toProcess.push({ &root, 0 });

  
  Debug::log(Stringf(
    "[ ]%-*s%-10s%-30s%-30s\n",
    57, "Function Name", "Call",
    option == SORT_TOTAL_TIME ? "--Total(Time)--" : "Total(Time)",
    option == SORT_SELF_TIME ? "--Self(Time)--" : "Self(Time)"));

  while (!toProcess.empty()) {
    Iter& top = toProcess.top();
    uint currentDepth = top.depth;

    // char str[1000];
    // sprintf_s(str, 1000, "%s", top.entry->name.data());

    // vec3 indentedPosition = position;
    // indentedPosition.x += font->advance(' ', ' ', kFontSize) * float(top.depth) ;

    Debug::log(Stringf(
      "%-*s%-*s%-10u%-30s%-30s",
      top.depth + 3, top.entry->children().empty() ? "   " : "[-]",
      57 - top.depth, top.entry->name.data(),
      top.entry->callCount, beautifySeconds(top.entry->totalTime).c_str(), beautifySeconds(top.entry->selfTime).c_str()));
    toProcess.pop();

    for (auto&[_, v] : top.entry->children()) {
      EXPECTS(v != nullptr);
      toProcess.push({ v, currentDepth + 1 });
    }
  }
}

double Report::totalFrameTime() const {
  return root.totalTime;
}

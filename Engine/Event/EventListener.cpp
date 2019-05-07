#include "EventListener.hpp"
#include "Engine/Event/EventEmitter.hpp"
#include <utility>

EventListener::~EventListener() {
}

void EventListener::subscribe(any_func&& func) {
  mHandles.emplace_back(func);
}

void EventListener::unsubscribe(any_func&& func) {
  for(size_t i = mHandles.size() - 1; i < mHandles.size(); --i) {
    if(mHandles[i] == func) {
      mHandles[i] = mHandles.back();
      std::swap(mHandles[i], mHandles.back());
      mHandles.pop_back();
    }
  }
}

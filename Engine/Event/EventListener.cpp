#include "EventListener.hpp"
#include "Engine/Event/EventEmitter.hpp"

EventListener::~EventListener() {
  for(auto& handle: mHandles) {
    mEmitter->freeCallback(handle.callback);
    handle.callback = nullptr;
  }
}

void EventListener::subscribe(void* origin, void* callback) {
  mHandles.push_back({ callback, origin });
}

void EventListener::unsubscribe(void* func) {
  for(size_t i = mHandles.size() - 1; i < mHandles.size(); --i) {
    if(mHandles[i]._originPointer == func) {
      mEmitter->freeCallback(mHandles[i].callback);
      std::swap(mHandles[i], mHandles.back());
      mHandles.pop_back();
    }
  }
}

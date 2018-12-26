#include "EventTarget.hpp"

void EventTarget::dispatch(std::string_view name, Event& e) {
  EventTarget* next = this;

  while(next != nullptr) {
    next->mEventEmitter.emit(name, e);
    if(e.cancelBubble) break;
    next = next->mParent;
  }

}

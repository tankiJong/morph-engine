#include "EventEmitter.hpp"


EventListener* EventEmitter::findListener(std::string_view name) const {
  auto iter = mEventListeners.find(name);
  return iter == mEventListeners.end() ? nullptr : iter->second;
}

EventListener* EventEmitter::findOrCreateListener(std::string_view name) {
  EventListener* listener = findListener(name);
  if(listener == nullptr) {
    listener = allocListener();
    listener->setName(name);
	  mEventListeners[std::string{name}] = listener;
  }
  return listener;
}

EventListener* EventEmitter::allocListener() {
  return new EventListener(this);
}

EventEmitter::EventEmitter() {
  // EventManager::bind(*this);
}

EventEmitter::~EventEmitter() {
  // EventManager::unbind(*this);
}

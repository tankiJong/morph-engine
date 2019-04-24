#include "EventEmitter.hpp"

EventEmitter& EventEmitter::on(std::string name, void* origin, void* cb) {

  EventListener* listener = findListener(name);
  if(listener == nullptr) {
    listener = allocListener();
    listener->setName(name);
    mEventListeners[name] = listener;
  }

  listener->subscribe(origin, cb);

  return *this;
}

EventListener* EventEmitter::findListener(std::string_view name) const {
  auto iter = mEventListeners.find(name);
  return iter == mEventListeners.end() ? nullptr : iter->second;
}

EventListener* EventEmitter::allocListener() {
  return new EventListener(this);
}

void EventEmitter::freeListener(EventListener* listener) {
  delete listener;
}

void* EventEmitter::allocCallback(size_t size) {
  return new byte_t[size];
}

void EventEmitter::freeCallback(void* callback) {
  byte_t* buf = (byte_t*) callback;
  delete[] buf;
}

EventEmitter::EventEmitter() {
  // EventManager::bind(*this);
}

EventEmitter::~EventEmitter() {
  // EventManager::unbind(*this);
}

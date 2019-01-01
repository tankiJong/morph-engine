#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Event/EventListener.hpp"
#include <map>
#include <functional>


class EventEmitter {
  friend class EventListener;
public:
  using EventCallback = void*;
  EventEmitter();
  ~EventEmitter();

  template<typename Func>
  EventEmitter& on(std::string name, Func&& func) {
    constexpr size_t cbSize = sizeof(decltype(std::function(func)));

    auto allocFunc = new(allocCallback(cbSize)) std::function(func);
    return on(name, &func, allocFunc, sizeof(allocFunc));
  };

  template<typename Func>
  EventEmitter& off(std::string name, Func&& func) {
    EventListener* listener = findListener(name);
    if(listener != nullptr) {
      listener->unsubscribe(&func);
    }
    return *this;
  }


  template<typename ...Args>
  bool emit(std::string_view name, Args ...args) {
    EventListener* listener = findListener(name);
    if(listener == nullptr) return false;
    return listener->invoke(args...);
  };

  EventListener* findListener(std::string_view name) const;
   
protected:
  EventEmitter& on(std::string name, void* origin, void* cb, size_t funcSize);
  EventListener* allocListener();
  void freeListener(EventListener* listener);

  void* allocCallback(size_t size);
  void freeCallback(void *callback);
  std::map<std::string, EventListener*, std::less<>> mEventListeners;
};


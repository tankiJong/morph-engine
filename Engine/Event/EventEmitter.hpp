#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Event/EventListener.hpp"
#include <map>
#include <functional>

/*
 * `EventEmitter` is a center where you can register/unregister event of different names
 */
class EventEmitter {
  friend class EventListener;
public:
  using EventCallback = void*;
  EventEmitter();
  ~EventEmitter();

  // ------------------------ on --------------------------
  EventEmitter& on(const std::string& name, any_func func) {
    EventListener* listener = findOrCreateListener(name);
    listener->subscribe(std::move(func));
    return *this;
  };

  template<typename Func>
  EventEmitter& on(std::string name, Func&& func) {
	  return on(name, any_func{func});
  };

  template<typename Func>
  EventEmitter& on(std::string name, void* object, Func&& func) {
    using func_t = function_t<Func>;
    return on(name, any_func{object, func});
  }

  // ------------------------ off --------------------------
  EventEmitter& off(const std::string& name, any_func func) {
    EventListener* listener = findListener(name);
    if(listener != nullptr) {
      listener->unsubscribe(std::move(func));
    }
    return *this;
  }

  template<typename Func>
  EventEmitter& off(std::string name, Func&& func) {
    return off(name, any_func{func});
  }

  template<typename Func>
  EventEmitter& off(std::string name, void* object, Func&& func) {
    return off(name, any_func{object, func});
  }

  // ------------------------ emit --------------------------
  template<typename ...Args>
  bool emit(std::string_view name, Args&& ...args) {
    EventListener* listener = findListener(name);
    if(listener == nullptr) return false;
    return listener->invoke(std::forward<Args>(args)...);
  };

   
protected:
  EventListener* findListener(std::string_view name) const;
  EventListener* findOrCreateListener(std::string_view name);
  EventListener* allocListener();
  void freeListener(EventListener* listener);

  std::map<std::string, EventListener*, std::less<>> mEventListeners;
};


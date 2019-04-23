#pragma once
#include "Engine/Core/common.hpp"
#include <vector>
#include <functional>

class EventEmitter;

/*
 * The `EventListener` is an entry of all callback of certain named event
 */
class EventListener {
  friend class EventEmitter;
public:
  EventListener(EventEmitter* owner) {
    mEmitter = owner;
  }

  ~EventListener();

  void setName(std::string_view name) {
    mName = name;
  }

  template<typename ...Args>
  bool invoke(Args ...args) const {
    for(CallbackHandle handle: mHandles) {
      std::function<void(Args...)>* func = (std::function<void(Args...)>*)handle.callback;
      (*func)(args...);
    }

    return !mHandles.empty();
  }

  void subscribe(void* origin, void* callback);

  void unsubscribe(void* func);

protected:
  struct CallbackHandle {
    void* callback;
    void* _originPointer;

    bool operator==(const CallbackHandle& hd) const {
      return _originPointer == hd._originPointer;
    }

    bool operator!=(const CallbackHandle& hd) const {
      return !(*this == hd);
    }
  };
  std::vector<CallbackHandle> mHandles;
  std::string mName;
  EventEmitter* mEmitter;
};

#pragma once
#include "Engine/Core/common.hpp"
#include <vector>
#include <functional>

class EventEmitter;

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
  bool invoke(Args ...args) {
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

    bool operator==(const CallbackHandle& hd) {
      return _originPointer == hd._originPointer;
    }

    bool operator!=(const CallbackHandle& hd) {
      return !(*this == hd);
    }
  };
  std::vector<CallbackHandle> mHandles;
  std::string mName;
  EventEmitter* mEmitter;
};

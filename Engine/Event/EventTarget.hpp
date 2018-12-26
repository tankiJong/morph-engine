#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Event/EventEmitter.hpp"

// some actual object, which can receive event

class EventTarget;

class Event {
  friend class EventTarget;
public:
  Event(): data(buffer) {}
  EventTarget* source = nullptr;
  void const* data;
private:
  bool cancelBubble = true;
  byte_t buffer[5 KB];
};

class EventTarget {
public:


  template<typename Callback>
  void bind(std::string name, Callback&& cb) {
    checkValidCallback<Callback>();
    mEventEmitter.on(name, cb);
  }

  template<typename Callback>
  void unbind(std::string name, Callback&& cb) {
    checkValidCallback<Callback>();
    mEventEmitter.off(cb);
  }

  void dispatch(std::string_view name, Event& e);


  EventTarget* parent() { return mParent; }
  EventTarget& parent(EventTarget* newParent) {
    mParent = newParent;
    return *this;
  }
protected:
  template<typename Callback>
  constexpr void checkValidCallback() {
    static_assert(std::is_invocable_v<Callback, Event>, 
                  "function should pass `Event` as Argument");
    static_assert(std::is_same_v<decltype(Callback(std::declval<Event>())), void>, 
                  "Callback function should return void"); 
  }
  EventTarget* mParent = nullptr;
  EventEmitter mEventEmitter;
};

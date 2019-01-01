#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Event/EventEmitter.hpp"

// some actual object, which can receive event

template<typename T>
class EventTarget;

template<typename T>
class Event {
  friend class EventTarget<T>;
public:
  EventTarget<T>* source = nullptr;
  T data;
private:
  bool cancelBubble = true;
};

template<typename D>
class EventTarget {
public:
  using Type = EventTarget<D>;
  using EventType = Event<D>;

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

  void dispatch(std::string_view name, Event<D>& e) const {
    Type* next = this;

    while(next != nullptr) {
      next->mEventEmitter.emit(name, e);
      if(e.cancelBubble) break;
      next = next->mParent;
    }

  };

  Type* parent() const { return mParent; }
  Type& parent(Type* newParent) {
    mParent = newParent;
    return *this;
  }

protected:
  template<typename Callback>
  constexpr void checkValidCallback() const {
    static_assert(std::is_invocable_v<Callback, Event<D>>, 
                  "function should pass `Event` as Argument");
    static_assert(std::is_same_v<decltype(Callback(std::declval<Event<D>>())), void>, 
                  "Callback function should return void"); 
  }
  Type* mParent = nullptr;
  EventEmitter mEventEmitter;
};

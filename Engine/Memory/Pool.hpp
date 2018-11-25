#pragma once
#include "Engine/Core/common.hpp"
#include <stack>
#include "Engine/Memory/Allocator.hpp"

template<typename T>
class Pool {
public:
  ~Pool() {
    while(!mFreeObjects.empty()) {
      T* ptr = mFreeObjects.top();
      delete ptr;
    }
  }

  template<typename ...Args>
  T* acquire(Args... args) {
    T* ptr = nullptr;
    if(mFreeObjects.empty()) {
      ptr = new T(args...);
    } else {
      ptr = mFreeObjects.top();
    }
    return ptr;
  }

  void release(T* obj) {
    mFreeObjects.push(obj);
  }

private:
  std::stack<T*> mFreeObjects;
};

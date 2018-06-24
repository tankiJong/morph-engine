#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/Fence.hpp"
#include <queue>

// Adpoted from Falcor, idea is an object pool managed by a fence.

template<typename T>
class FencedPool: public std::enable_shared_from_this<FencedPool<T>> {
public:
  using sptr_t = S<FencedPool<T>>;
  using scptr_t = S<const FencedPool<T>>;
  using new_obj_func_t = T(*)(void*);

  static sptr_t create(Fence::scptr_t fence, new_obj_func_t func, void* userData = nullptr) {
    return sptr_t(new FencedPool(fence, func, userData));
  }

  T aquire() {
    data_t data;
    data.alloc = mActiveObj;
    data.timestamp = mFence->cpuValue();
    mQueue.push(data);

    // The queue is sorted based on time. Check if the first object is free
    data = mQueue.front();
    if(data.timestamp <= mFence->gpuVaule()) {
      mQueue.pop();
    } else {
      data.alloc = mNewObjFunc(mUserData);
    }
    mActiveObj = data.alloc;
    return mActiveObj;
  }

protected:
  FencedPool(Fence::scptr_t fence, new_obj_func_t func, void* userData = nullptr)
    : mFence(fence), mNewObjFunc(func), mUserData(userData) {
    mActiveObj = mNewObjFunc(userData);
  }

  struct data_t {
    T alloc;
    u64 timestamp;
  };

  Fence::scptr_t mFence;
  void* mUserData;
  T mActiveObj;
  new_obj_func_t mNewObjFunc;
  std::queue<data_t> mQueue;
};
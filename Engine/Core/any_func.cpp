#include "any_func.hpp"

any_func::any_func(any_func&& func) noexcept
  : mCallback(func.mCallback)
  , mObject(func.mObject)
  , mArgInfo(func.mArgInfo)
  , mFunctionInfo(func.mFunctionInfo)
  , mFunction(std::move(func.mFunction)) {
  func.mCallback = nullptr;
  func.mObject   = nullptr;
  func.mArgInfo  = nullptr;
}

any_func::any_func(const any_func& func)
  : mCallback(func.mCallback)
  , mObject(func.mObject)
  , mArgInfo(func.mArgInfo)
  , mFunctionInfo(func.mFunctionInfo)
  , mFunction(func.mFunction) {}

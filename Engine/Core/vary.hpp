#pragma once
#include "Engine/Core/common.hpp"
#include <any>
#include "Engine/Debug/ErrorWarningAssert.hpp"

struct vary {
  template<typename T>
  T& get() const;

  template<typename T, typename V = std::decay_t<T>>
  void set(T&& value);

  ~vary();
protected:
  using deleter_t = void(*)(vary&);
  struct storage_t{
    static constexpr size_t kBufferSize = 32 - sizeof(void*);
    uint32_t buf[kBufferSize];
  }; // 124 bytes

  union {
    storage_t     mStorage;
    struct {
      void* value = nullptr;
    }             mPtr{};
  };

      deleter_t   mDeleter = nullptr;
  const unique*   mTypeInfo = nullptr;

  void reset();

  template<typename T, bool UseStorage>
  void destructor(vary& v);
};

template< typename T >
inline T& vary::get() const {
  EXPECTS(&tid<T>::value == mTypeInfo);

  constexpr size_t vsize = sizeof(T);

  T* valptr = nullptr;

  if constexpr ( vsize > sizeof(storage_t)) {
    // use heap storage
    EXPECTS(mPtr.value != nullptr);
    valptr = (T*)mPtr.value;
  } else {
    // use local storage
    valptr = (T*)&mStorage;
  }

  return *valptr;
}

template< typename T, typename V>
inline void vary::set(T&& value) {
  constexpr size_t vsize = sizeof(V);

  reset();
  V* valptr = nullptr;
  if constexpr ( vsize > sizeof(storage_t)) {
    // use heap storage
    valptr = (V*)malloc(vsize);
    mPtr.value = valptr;
    mDeleter = &destructor<V, true>;
  } else {
    // use local storage
    valptr = (V*)&mStorage;
    mDeleter = &destructor<V, false>;
  }

  // finally emplace new.
  new (valptr) V(value);
  mTypeInfo = &tid<V>::value;
}

template< typename T, bool UseStorage >
void vary::destructor(vary& v) {
  T* vptr = nullptr;

  if constexpr (std::is_destructible_v<T>) {
    if constexpr (UseStorage) {
      vptr = &mStorage;
    } else {
      vptr = mPtr.value;
    }
    vptr->~T();
  }

  if constexpr (!UseStorage) {
   free(v.mPtr.value);
    v.mPtr.value = nullptr;
    v.mDeleter = nullptr;
  }

}


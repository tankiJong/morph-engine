#pragma once
#include "Engine/Core/common.hpp"
#include <any>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include <map>

struct vary {
  vary() {};
  
  template<typename T>
  const T& get() const;
  template<typename T>
  T& get();

  const void* get() const;
  void* get();

  template<typename T, typename V = std::decay_t<T>>
  void set(T&& value);

  vary(vary&& from) noexcept;
  vary(const vary& from);

  vary& operator=(const vary& from);
  vary& operator=(vary&& from) noexcept;

  bool operator==(const vary& rhs) const {
    if(rhs.mMetaData != mMetaData) return false;

    if(mMetaData.useHeap) {
      return mPtr == rhs.mPtr;
    } else {
      return memcmp(&mStorage, &rhs.mStorage, storage_t::kBufferSize);
    }
  }

  ~vary();
protected:
  template<typename T>
  static void copyConstruct(const void* from, void* to);
  template<typename T, bool UseHeap>
  static void destructor(vary& v);

  static void defaultDelete(vary&) {}
  static void defaultCopy(const void*, void*) {}

  using deleter_t = void(*)(vary&);
  using copy_construct_t = void(*)(const void*, void*);
  struct meta_data_t {
    deleter_t         deleter  = &vary::defaultDelete;
    copy_construct_t  copyConstructor = &vary::defaultCopy;
    const unique*     typeInfo = nullptr;
    bool              useHeap  = false;

    bool operator==(const meta_data_t& rhs) const {
      return deleter == rhs.deleter &&
             copyConstructor == rhs.copyConstructor &&
             typeInfo == rhs.typeInfo &&
             useHeap == rhs.useHeap;
    }

    bool operator!=(const meta_data_t& rhs) const {
      return !((*this) == rhs);
    }
    void reset();
  };

  struct storage_t{
    static constexpr size_t kBufferSize = 256  - sizeof(meta_data_t);
    uint8_t buf[kBufferSize];
  };

  union {
    storage_t     mStorage;
    struct blob {
      void*  value = nullptr;
      size_t size  = 0;

      bool operator==(const blob& rhs) const {
        return size == rhs.size && memcmp(value, rhs.value, size) == 0;
      }
    }             mPtr{};
  };

  meta_data_t mMetaData;

  void reset();

};

template< typename T >
inline const T& vary::get() const {
  EXPECTS(&tid<T>::value == mMetaData.typeInfo);
  T* valptr = (T*)get();
  return *valptr;
}

template< typename T >
T& vary::get() {
  EXPECTS(&tid<T>::value == mMetaData.typeInfo);
  T* valptr = (T*)get();
  return *valptr;
}

template< typename T, typename V>
inline void vary::set(T&& value) {
  constexpr size_t vsize = sizeof(V);

  reset();
  V* valptr = nullptr;

  constexpr bool useHeap = vsize > sizeof(storage_t);
  //memset(&mStorage, 0, sizeof(storage_t));
  if constexpr (useHeap) {
    // use heap storage
    valptr = (V*)malloc(vsize);
    mPtr.value = valptr;
    mPtr.size = vsize;
  } else {
    // use local storage
    valptr = (V*)&mStorage;
  }
  
  // finally emplace new.
  new (valptr) V(value);

  mMetaData = { &destructor<V, useHeap>, &copyConstruct<V>, &tid<V>::value, useHeap };
}

template< typename T >
void vary::copyConstruct(const void* from, void* to) {
  T* vFrom = (T*)from;
  new (to) T(*vFrom);
}

template< typename T, bool UseHeap >
void vary::destructor(vary& v) {
  T* vptr = nullptr;

  if constexpr (std::is_destructible_v<T>) {
    if constexpr (UseHeap) {
      vptr = (T*)v.mPtr.value;
    } else {
      vptr = (T*)&v.mStorage;
    }
    vptr->~T();
  }

  if constexpr (UseHeap) {
    free(v.mPtr.value);
    v.mPtr.value = nullptr;
  }

}

template<typename T, typename V = std::decay_t<T>>
void operator << (T&& lhs, const vary& rhs) {
  lhs = rhs.get<V>();
}

class VaryMap {
public:

  template<typename T>
  void set(std::string name, T&& value) {
    mMap[name].set(value);
  }

  template<typename T>
  T& get(std::string_view name) {
    auto kv = mMap.find(name);
    EXPECTS(kv != mMap.end());
    return kv->second.get<T>();
  }

  template<typename T>
  bool get(std::string_view name, T& value) {
    auto kv = mMap.find(name);
    if(kv == mMap.end()) return false;
    value = kv->second.get<T>();
    return true;
  }

protected:
  std::map<std::string, vary, std::less<>> mMap;
};
#pragma once
#include <memory>
#include "Engine/Core/common.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

class Blob {
public:
  template<typename T>
  Blob(T* source, size_t size): buffer(malloc(size)), dataSize(size), bufferSize(size) {
    memcpy_s(buffer, size, source, size);
  }

  Blob(size_t size): buffer(malloc(size)), dataSize(0), bufferSize(size) {}

  Blob(): buffer(malloc(0)), dataSize(0), bufferSize(0) {}

  Blob(Blob& b) = delete;
  ~Blob();
  Blob(Blob&& source) noexcept;

  Blob clone() const;

  void set(const void* data, size_t size, size_t offset = 0);
  template<typename T>
  operator T() {
     return as<T>();
  }

  template<typename T>
  operator const T() const {
    return as<T>();
  }

  operator bool() {
    return valid();
  }

  operator const bool() const {
    return valid();
  }


  Blob& operator=(Blob&& other) noexcept;

  template<typename T>
  T as() {
    static_assert(std::is_pointer<T>::value, "T has to be pointer type");
    return reinterpret_cast<T>(buffer);
  }

  template<typename T>
  const T as() const {
    static_assert(std::is_pointer<T>::value, "T has to be pointer type");
    return reinterpret_cast<const T>(buffer);
  }

  inline bool valid() const { return dataSize != 0; };
  inline size_t size() const { return dataSize; };
  inline size_t capacity() const { return bufferSize; };
protected:
  void* buffer = nullptr;
  size_t dataSize;
  size_t bufferSize;

};

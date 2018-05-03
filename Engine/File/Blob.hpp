#pragma once
#include <memory>
#include "Engine/Core/common.hpp"
class Blob {
public:
  template<typename T>
  Blob(T* source, unsigned size): buffer(malloc(size)), dataSize(size), bufferSize(size) {
    memcpy_s(buffer, size, source, size);
  }

  Blob(unsigned size): buffer(malloc(size)), dataSize(0), bufferSize(size) {}

  Blob(): buffer(malloc(0)), dataSize(0), bufferSize(0) {}

  Blob(Blob& b) = delete;
  ~Blob();
  Blob(Blob&& source) noexcept;

  Blob clone() const;

  void set(void* data, size_t size, size_t offset = 0);
  template<typename T>
  operator T() {
     return as<T>();
  }

  template<typename T>
  operator const T() const {
    return as<T>();
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

  inline unsigned size() const { return dataSize; };
  inline unsigned capacity() const { return bufferSize; };
protected:
  void* buffer = nullptr;
  unsigned dataSize;
  unsigned bufferSize;

};

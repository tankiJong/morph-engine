#pragma once
#include <type_traits>
#include <corecrt_memcpy_s.h>

class Blob {
public:
  void* buffer = nullptr;
  unsigned size;

  template<typename T>
  Blob(T* source, unsigned size): buffer(source), size(size) {}

  template<typename T>
  Blob(const T* source, unsigned _size): size(_size) {
    buffer = malloc(size);
    memcpy_s(buffer, size, source, size);
  }

  Blob(): buffer(nullptr), size(0) {}

  Blob(Blob& b) = delete;
  ~Blob();
  Blob(Blob&& source) noexcept;

  Blob clone() const;

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
};

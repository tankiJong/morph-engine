#pragma once
#include <type_traits>

#include "ThirdParty/gsl/span"
#include <memory>

// source GSL: https://github.com/Microsoft/GSL/blob/master/include/gsl/pointers

//
// owner
//
// owner<T> is designed as a bridge for code that must deal directly with owning pointers for some reason
//
// T must be a pointer type
// - disallow construction from any type other than pointer type
//
template <class T, class = std::enable_if_t<std::is_pointer<T>::value>>
using owner = T;

template <class ElementType, std::ptrdiff_t Extent = gsl::dynamic_extent>
using span = gsl::span<ElementType, Extent>;


struct unique {
  inline bool operator=(const unique& rhs) const { return id() == rhs.id(); }
protected:
  virtual inline size_t id() const= 0;
};

template<typename T>
struct tid : public unique {
protected:
  inline size_t id() const { return (size_t)&_id; }
  static char _id;
};

template<typename T>
char tid<T>::_id = 0;


template<typename T>
using S = std::shared_ptr<T>;

#define SAFE_DELETE(p) if((p)) { delete (p); (p) = nullptr; }

#define ___APPEND_IMPL(a, b) a##b
#define APPEND(a, b) ___APPEND_IMPL(a, b)

#define BIT_FLAG(f) (1U<<(f))

#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)
#define enum_class_operators(e_) inline e_ operator& (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b));}  \
    inline e_ operator| (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b));} \
    inline e_& operator|= (e_& a, e_ b){a = a | b; return a;};  \
    inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; };   \
    inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a));}   \
    inline bool is_set(e_ val, e_ flag) { return (val & flag) != (e_)0;}


#define KB *1024
#define MB *1024*KB
#define GB *1024*MB
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

template<typename T>
using U = std::unique_ptr<T>;

template<typename T>
using W = std::weak_ptr<T>;

#define SAFE_DELETE(p) if((p)) { delete (p); (p) = nullptr; } 
#define count_of(arr) sizeof(arr) / sizeof(arr[0])

#define ___APPEND_IMPL(a, b) a##b
#define APPEND(a, b) ___APPEND_IMPL(a, b)

// Adopted from Falcor

// This is a helper class which should be used in case a class derives from a base class which derives from enable_shared_from_this
// If Derived will also inherit enable_shared_from_this, it will cause multiple inheritance from enable_shared_from_this, which results in a runtime errors because we have 2 copies of the WeakPtr inside shared_ptr
template<typename Base, typename Derived>
class inherit_shared_from_this {
public:
  typename std::shared_ptr<Derived> shared_from_this() {
    Base* pBase = static_cast<Derived*>(this);
    std::shared_ptr<Base> pShared = pBase->shared_from_this();
    return std::static_pointer_cast<Derived>(pShared);
  }

  typename std::shared_ptr<const Derived> shared_from_this() const {
    const Base* pBase = static_cast<const Derived*>(this);
    std::shared_ptr<const Base> pShared = pBase->shared_from_this();
    return std::static_pointer_cast<const Derived>(pShared);
  }
};
#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)
#define enum_class_operators(e_) inline e_ operator& (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b));}  \
    inline e_ operator| (e_ a, e_ b){return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b));} \
    inline e_& operator|= (e_& a, e_ b){a = a | b; return a;};  \
    inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; };   \
    inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a));}   \
    inline bool is_set(e_ val, e_ flag) { return (val & flag) != (e_)0;}




#pragma once
#include <type_traits>

#include "ThirdParty/gsl/span"

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
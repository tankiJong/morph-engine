#pragma once
#include <vector>
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/type.h"

struct VertexAttribute {
  std::string name;
  eDataDeclType type = MP_FLOAT;
  uint count = 1;
  uint offset = 0;
  bool isNormalized = false;

  VertexAttribute(std::string name, eDataDeclType type, uint count, uint offset, bool isNormalized = true);
};

class VertexLayout {
public:
  VertexLayout() = default;
  
  void attribute(std::string name, eDataDeclType type, uint count, uint offset, bool isNormalized = true);
protected:
  std::vector<VertexAttribute> mAttribs;

  template<typename VertexType>
  static inline const VertexLayout* For();
};

namespace detail {
  template<typename VertexType>
  class __VertexLayoutConstructor: public VertexLayout {
  public:
    __VertexLayoutConstructor() { construct(); }
  protected:
    void construct();
  };
}

#define DeclVertexType( name ) \
  struct name; \
  template<> inline const VertexLayout* VertexLayout::For<name>() { \
    static detail::__VertexLayoutConstructor<name> format;\
    return &format; \
  } \
  struct name

#define DefineVertexType( name ) \
  template<> void detail::__VertexLayoutConstructor<name>::construct() 
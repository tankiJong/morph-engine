#pragma once
#include <vector>
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHI.hpp"

class VertexBuffer;

struct VertexAttribute {
  uint streamIndex = 0;
  std::string name;
  eDataDeclType type = MP_FLOAT;
  uint count = 1;
  uint offset = 0;
  bool isNormalized = false;
  uint offsetInVertexArray;
  VertexAttribute(uint streamIndex, std::string name, eDataDeclType type, uint count, uint offset, bool isNormalized, uint offsetInVertexArray);

  void initVertexBuffer(VertexBuffer& vbo) const;

  uint stride() const;
};

class VertexLayout {
public:
  VertexLayout() = default;
  
  void define(uint streamIndex, std::string name, eDataDeclType type, uint count, uint offset, bool isNormalized, uint offsetInVertexArray);

  inline span<const VertexAttribute> attributes() const { return mAttribs; }
  
  template<typename VertexType>
  static inline const VertexLayout* For();

  template<typename VertexType>
  static inline constexpr bool Valid() { return false;}
protected:
  std::vector<VertexAttribute> mAttribs;
  std::string mVertexType;
};

namespace detail {
  template<typename VertexType>
  class __VertexLayoutConstructor: public VertexLayout {
  public:
    __VertexLayoutConstructor(const char* name) {
      mVertexType = name; 
      construct();
    }
  protected:
    void construct();
  };
}

#define DeclVertexType( name ) \
  struct name; \
  template<> inline constexpr bool VertexLayout::Valid<name>() { return true; } \
  template<> inline const VertexLayout* VertexLayout::For<name>() { \
    static detail::__VertexLayoutConstructor<name> format(#name);\
    return &format; \
  } \
  struct name

#define DefineVertexType( name ) \
  template<> void detail::__VertexLayoutConstructor<name>::construct() 
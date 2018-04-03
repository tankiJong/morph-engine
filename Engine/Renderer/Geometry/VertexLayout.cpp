#include "VertexLayout.hpp"
VertexAttribute::VertexAttribute(std::string name, eDataDeclType type, uint count, uint offset, bool isNormalized)
  : name(name)
  , type(type)
  , count(count)
  , offset(offset)
  , isNormalized(isNormalized) {
    
}

void VertexLayout::define(std::string name, eDataDeclType type, uint count, uint offset, bool isNormalized) {
  mAttribs.emplace_back(name, type, count, offset, isNormalized);
}

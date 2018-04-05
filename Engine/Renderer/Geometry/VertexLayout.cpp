#include "VertexLayout.hpp"
#include "VertexBuffer.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

VertexAttribute::VertexAttribute(uint streamIndex, std::string name, eDataDeclType type, uint count, uint offset, bool isNormalized, uint offsetInVertexArray)
  : streamIndex(streamIndex)
  , name(name)
  , type(type)
  , count(count)
  , offset(offset)
  , isNormalized(isNormalized)
  , offsetInVertexArray(offsetInVertexArray){
    
}

void VertexAttribute::initVertexBuffer(VertexBuffer& vbo) const {

  vbo.vertexStride = stride();
}

uint VertexAttribute::stride() const {
  uint size = 0;
  switch (type) {
    case MP_FLOAT:
      size = sizeof(float);
      break;
    case MP_BYTE:
    case MP_UBYTE:
      size = sizeof(char);
      break;
    default:
      ERROR_AND_DIE("unspoorted type");
  }

  return size * count;
}

void VertexLayout::define(uint index, std::string name, eDataDeclType type, uint count, uint offset, bool isNormalized, uint offsetInVertexArray) {
  mAttribs.emplace_back(index, name, type, count, offset, isNormalized, offsetInVertexArray);
}

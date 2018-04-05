#include "Vertex.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

DefineVertexType(vertex_pcu_t) {
  define(0, "POSITION", MP_FLOAT, 3, 0, true, offsetof(vertex_a_t, position) /sizeof(size_t));
  define(1, "COLOR",    MP_UBYTE, 4, 0, false, offsetof(vertex_a_t, color) / sizeof(size_t));
  define(2, "UV",       MP_FLOAT, 2, 0, true, offsetof(vertex_a_t, uv) / sizeof(size_t));
}

//template<>
//vertex_pcu_t Vertex::as() {
//  return {
//    position.data(),
//    color,
//    uv
//  };
//}


Vertex::Vertex() {
  mPositions.reserve(100u);
  mUVs.reserve(100u);
  mColors.reserve(100u);
}

vertex_a_t Vertex::vertices() {
  vertex_a_t v;
  v.position = mPositions.data();
  v.uv = mUVs.data();
  v.color = mColors.data();

  return v;
}

void Vertex::push(vertex_t v) {
  mPositions.push_back(v.position);
  mColors.push_back(v.color);
  mUVs.push_back(v.uv);
  
  mCount++;
}

void Vertex::reserve(uint size) {
  mPositions.reserve(size);
  mColors.reserve(size);
  mUVs.reserve(size);
}

void Vertex::clear() {
  mPositions.clear();
  mColors.clear();
  mUVs.clear();

  mCount = 0;
}

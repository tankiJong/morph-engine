#include "Vertex.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

DefineVertexType(vertex_pcu_t) {
  define(0, "POSITION", MP_FLOAT, 3, 0, true, offsetof(vertex_a_t, position) /sizeof(size_t));
  define(1, "COLOR",    MP_FLOAT, 4, 0, false, offsetof(vertex_a_t, color) / sizeof(size_t));
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

DefineVertexType(vertex_lit_t) {
  define(0, "POSITION", MP_FLOAT, 3, 0, true, offsetof(vertex_a_t, position) / sizeof(size_t));
  define(1, "COLOR", MP_FLOAT, 4, 0, false, offsetof(vertex_a_t, color) / sizeof(size_t));
  define(2, "UV", MP_FLOAT, 2, 0, true, offsetof(vertex_a_t, uv) / sizeof(size_t));
  define(3, "NORMAL", MP_FLOAT, 3, 0, true, offsetof(vertex_a_t, normal) / sizeof(size_t));
  define(4, "TANGENT", MP_FLOAT, 4, 0, true, offsetof(vertex_a_t, tangent) / sizeof(size_t));
}


Vertex::Vertex() {
  reserve(1000u);
}

vertex_a_t Vertex::vertices() {
  vertex_a_t v{};
  v.position = mPositions.data();
  v.uv = mUVs.data();
  v.color = mColors.data();
  v.normal = mNormals.data();
  v.tangent = mTangents.data();
  return v;
}

void Vertex::push(const vertex_t& v) {
  mPositions.emplace_back(v.position);
  mColors.emplace_back(v.color);
  mUVs.emplace_back(v.uv);
  mNormals.emplace_back(v.normal);
  mTangents.emplace_back(v.tangent);
  mCount++;
}

void Vertex::reserve(uint size) {
  mPositions.reserve(size);
  mColors.reserve(size);
  mUVs.reserve(size);
  mNormals.reserve(size);
  mTangents.reserve(size);
}

void Vertex::clear() {
  mPositions.clear();
  mColors.clear();
  mUVs.clear();
  mNormals.clear();
  mTangents.clear();
  mCount = 0;
}

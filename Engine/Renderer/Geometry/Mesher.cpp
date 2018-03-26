#include "Mesher.hpp"
#include "Mesh.hpp"
#include "gsl/span"
Mesher& Mesher::begin(eDrawPrimitive prim, bool useIndices) {
  mIns.prim = prim;
  mIns.useIndices = useIndices;

  if(useIndices) {
    mIns.startIndex = mIndices.size();
  } else {
    mIns.startIndex = mVertices.size();
  }

  isDrawing = false;
  return *this;
}

void Mesher::end() {
  uint end;

  if(mIns.useIndices) {
    end = mIndices.size();
  } else {
    end = mVertices.size();
  }
  mIns.elementCount = end - mIns.startIndex;
  isDrawing = false;
}

void Mesher::clear() {}
Mesher& Mesher::color(const Rgba& c) {
  mStamp.color = c;
  return *this;
}

Mesher& Mesher::uv(const vec2& uv) {
  mStamp.uv = uv;
  return *this;
}

uint Mesher::vertex3f(const vec3& pos) {
  mStamp.position = pos;
  mVertices.push_back(mStamp);
  return mVertices.size() - 1u;
}

uint Mesher::vertex3f(float x, float y, float z) {
  mStamp.position.x = x;
  mStamp.position.y = y;
  mStamp.position.z = z;

  mVertices.push_back(mStamp);
  return mVertices.size() - 1u;
}

uint Mesher::vertex2f(const vec2& pos) {
  return vertex3f(pos.x, pos.y, 0);
}

Mesher& Mesher::triangle(uint a, uint b, uint c) {
  mIndices.push_back(a);
  mIndices.push_back(b);
  mIndices.push_back(c);
  mIns.elementCount += 3;
  return *this;
}

owner<Mesh*> Mesher::createMesh() {
  GUARANTEE_OR_DIE(isDrawing == false, "createMesh called without calling end()");
  Mesh* m = new Mesh(sizeof(Vertex));
  m->setInstruction(mIns.prim, mIns.useIndices, mIns.startIndex, mIns.elementCount);

  m->setVertices<Vertex>(mVertices);
  if(mIns.useIndices) {
    m->setIndices({ mIndices.data() + mIns.startIndex, mIndices.data() + mIns.elementCount});
  }

  return m;
}

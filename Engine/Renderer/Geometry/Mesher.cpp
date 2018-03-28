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

  isDrawing = true;
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

void Mesher::clear() {
  mVertices.clear();
  mIndices.clear();
}
Mesher& Mesher::color(const Rgba& c) {
  mStamp.color = c;
  return *this;
}

Mesher& Mesher::uv(const vec2& uv) {
  mStamp.uv = uv;
  return *this;
}

Mesher& Mesher::uv(float u, float v) {
  mStamp.uv.x = u;
  mStamp.uv.y = v;
  return *this;
}

uint Mesher::vertex3f(const vec3& pos) {
  mStamp.position = pos;
  mVertices.push_back(mStamp);
  return mVertices.size() - 1u;
}

uint Mesher::vertex3f(span<const vec3> verts) {
  mVertices.reserve(mVertices.size() + verts.size());
  for(const vec3& vert: verts) {
    vertex3f(vert);
  }
  return mVertices.size() - verts.size();
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

Mesher& Mesher::line(const vec3& from, const vec3& to) {
  uint start = vertex3f(from);
  vertex3f(to);

  mIndices.push_back(start);
  mIndices.push_back(start + 1);

  return *this;
}

Mesher& Mesher::sphere(const vec3& center, float size, uint levelX, uint levelY) {
//  mIns.prim = DRAW_POINTS;
//  mIns.useIndices = false;
  float dTheta = 360.f / (float)levelX;
  float dPhi = 180.f / (float)levelY;

  uint start = mVertices.size();

  for (uint j = 0; j <= levelY; j++) {
    for (uint i = 0; i <= levelX; i++) {
      float phi = dPhi * (float)j - 90.f, theta = dTheta * (float)i;
      vec3 pos = fromSpherical(size, theta, phi) + center;
      uv(theta / 360.f, (phi + 90.f) / 180.f);
      vertex3f(pos);
    }
  }

  for(uint j = 0; j < levelY; j++) {
    for(uint i = 0; i < levelX; i++) {
      uint current = j * (levelX + 1) + i;
//      triangle(current, current + 1, current + levelX + 1);
//      triangle(current, current + levelX + 1, current + levelX);
      quad(current, current + 1, current + (levelX + 1)+1, current + (levelX + 1));
    }
  }

  return *this;
}

Mesher& Mesher::triangle(uint a, uint b, uint c) {
  switch(mIns.prim) {
    case DRAW_POINTS:
    case DRAW_TRIANGES: {
      mIndices.push_back(a);
      mIndices.push_back(b);
      mIndices.push_back(c);
    } break;
    case DRAW_LINES: {
      mIndices.push_back(a);
      mIndices.push_back(b);
      mIndices.push_back(b);
      mIndices.push_back(c);
      mIndices.push_back(c);
      mIndices.push_back(a);
    } break;
    default:
      ERROR_AND_DIE("unsupported primitive");
  }

  return *this;
}

Mesher& Mesher::quad(uint a, uint b, uint c, uint d) {
  switch(mIns.prim) {
    case DRAW_POINTS: {
      mIndices.push_back(a);
      mIndices.push_back(b);
      mIndices.push_back(c);
      mIndices.push_back(d);
    } break;
    case DRAW_LINES: {
      mIndices.push_back(a);
      mIndices.push_back(b);
      mIndices.push_back(b);
      mIndices.push_back(c);
      mIndices.push_back(c);
      mIndices.push_back(d);
      mIndices.push_back(d);
      mIndices.push_back(a);
    } break;
    case DRAW_TRIANGES: {
      triangle(a, b, c);
      triangle(a, c, d);
    } break;
    default: 
      ERROR_AND_DIE("unsupported primitive");
  }

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

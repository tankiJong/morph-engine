#include "Mesher.hpp"
#include "Mesh.hpp"
#include "gsl/span"
Mesher& Mesher::begin(eDrawPrimitive prim, bool useIndices) {
  GUARANTEE_OR_DIE(isDrawing == false, "Call begin before previous end get called.");
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
  GUARANTEE_OR_DIE(isDrawing, "Call end without calling begin before");
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
  mIns = draw_instr_t();
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
      uint current = start + j * (levelX + 1) + i;
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

Mesher& Mesher::quad(const vec3& a, const vec3& b, const vec3& c, const vec3& d) {
  uint start =
  uv({ 0,0 })
    .vertex3f(a);

  uv({ 1,0 })
    .vertex3f(b);

  uv({ 1,1 })
    .vertex3f(c);

  uv({ 0,1 })
    .vertex3f(d);
  quad(start + 0, start + 1, start + 2, start + 3);

  return *this;
}

Mesher& Mesher::cube(const vec3& center, const vec3& dimension) {
  vec3 bottomCenter = center - vec3::up * dimension.y * .5f;
  float dx = dimension.x * .5f, dy = dimension.y * .5f, dz = dimension.z * .5f;

  std::array<vec3, 8> vertices = {
    bottomCenter + vec3{ -dx, 2.f * dy, -dz },
    bottomCenter + vec3{ dx, 2.f * dy, -dz },
    bottomCenter + vec3{ dx, 2.f * dy,  dz },
    bottomCenter + vec3{ -dx, 2.f * dy,  dz },

    bottomCenter + vec3{ -dx, 0, -dz },
    bottomCenter + vec3{ dx, 0, -dz },
    bottomCenter + vec3{ dx, 0,  dz },
    bottomCenter + vec3{ -dx, 0,  dz }
  };

  quad(vertices[0], vertices[1], vertices[2], vertices[3]);
  quad(vertices[4], vertices[5], vertices[6], vertices[7]);
  quad(vertices[4], vertices[5], vertices[1], vertices[0]);
  quad(vertices[5], vertices[6], vertices[2], vertices[1]);
  quad(vertices[6], vertices[7], vertices[3], vertices[2]);
  quad(vertices[7], vertices[4], vertices[0], vertices[3]);

  return *this;
}
#pragma once
#include "Engine/Core/common.hpp"
#include "Game/Game.hpp"
#include "Vertex.hpp"
#include "type.h"

class Mesh;

class Mesher {
public:
  Mesher& begin(eDrawPrimitive prim, bool useIndices = true);
  void end();
  void clear();
  Mesher& color(const Rgba& c);
  Mesher& uv(const vec2& uv);
  Mesher& uv(float u, float v);

  uint vertex3f(const vec3& pos);
  uint vertex3f(span<const vec3> verts);
  uint vertex3f(float x, float y, float z);
  uint vertex2f(const vec2& pos);

  Mesher& line(const vec3& from, const vec3& to);
  Mesher& sphere(const vec3& center, float size, uint levelX = 10u, uint levelY = 10u);
  Mesher& triangle(uint a, uint b, uint c);
  Mesher& quad(uint a, uint b, uint c, uint d);
  Mesher& quad(const vec3& a, const vec3& b, const vec3& c, const vec3& d);
  Mesher& cube(const vec3& center, const vec3& dimension);
  owner<Mesh*> createMesh();

protected:
  Vertex mStamp;
  std::vector<Vertex> mVertices;
  std::vector<uint> mIndices;
  draw_instr_t mIns;
  bool isDrawing = false;
};

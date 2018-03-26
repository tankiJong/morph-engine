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

  uint vertex3f(const vec3& pos);
  uint vertex3f(float x, float y, float z);

  uint vertex2f(const vec2& pos);

  Mesher& triangle(uint a, uint b, uint c);

  owner<Mesh*> createMesh();

protected:
  Vertex mStamp;
  std::vector<Vertex> mVertices;
  std::vector<uint> mIndices;
  draw_instr_t mIns;
  bool isDrawing = false;
};

#pragma once
#include "Engine/Renderer/type.h"

struct draw_instr_t {
  eDrawPrimitive prim = DRAW_TRIANGES;
  bool useIndices = true;
  uint startIndex = 0;
  uint elementCount = 0;
};



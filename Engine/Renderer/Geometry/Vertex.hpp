#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Core/Rgba.hpp"

class Vertex {
public:
  vec3 position;
  Rgba color;
  vec2 uv;
};

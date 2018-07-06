#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Debug/Profile/Report.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include "Engine/Math/Range.hpp"
#include "Engine/Core/Gradient.hpp"
#include "Engine/Debug/Profile/Profiler.hpp"
#include "Engine/Renderer/Renderable/Renderable.hpp"
#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Math/Transform.hpp"

namespace Profile {
  class Report;

  void updateOverlay();
  void renderOverlay();
  void initOverlay();
}

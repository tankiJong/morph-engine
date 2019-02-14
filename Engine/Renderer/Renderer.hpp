#pragma once

#include "Engine/Core/common.hpp"

class RHIContext;
class FrameBuffer;
class Renderer {
public:
  Renderer() = default;
  virtual ~Renderer() {};

  virtual void onLoad(RHIContext& ctx) { UNUSED(ctx); }
  virtual void onRenderFrame(RHIContext& ctx) { UNUSED(ctx); }
  virtual void onRenderGui(RHIContext& ctx /*, Gui* gui*/) { UNUSED(ctx); }

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer &) = delete;
};

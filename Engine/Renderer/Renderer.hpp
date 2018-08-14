#pragma once

#include "Engine/Core/common.hpp"

class RHIContext;
class FrameBuffer;
class Renderer {
public:
  Renderer() = default;
  virtual ~Renderer() {};

  virtual void onLoad(RHIContext& ctx) {}
  virtual void onRenderFrame(RHIContext& ctx, FrameBuffer& fbo) {}
  virtual void onRenderGui(RHIContext& ctx /*, Gui* gui*/) {}

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer &) = delete;
};

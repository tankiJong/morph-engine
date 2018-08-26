#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/RHI/FrameBuffer.hpp"
#include "Engine/Graphics/RHI/TypedBuffer.hpp"

// deferred renderer

class SceneRenderer : public Renderer {
public:
  SceneRenderer(const RenderScene& target);
  ~SceneRenderer() override;

  void onLoad(RHIContext& ctx) override;

  void onRenderFrame(RHIContext& ctx) override;

  void onRenderGui(RHIContext& ctx) override;

protected:
  void genGBuffer(RHIContext& ctx);
  void setupFrame();
  void setupView(RHIContext& ctx);
  const RenderScene& mTargetScene;

  FrameBuffer mGFbo;
  // G-Buffers
  Texture2::sptr_t mGAlbedo;
  Texture2::sptr_t mGNormal;
  Texture2::sptr_t mGDepth;

  Texture2::sptr_t mAO;

  Texture2::sptr_t mFinalColor;

  // cbo
  frame_data_t mFrameData;
  S<RHIBuffer> mcFrameData;
  S<RHIBuffer> mcCamera;
  S<RHIBuffer> mcModel;
  TypedBuffer::sptr_t mcLight;

};

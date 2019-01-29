#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/RenderGraph/RenderGraph.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"

class RenderPass;
class RenderScene;

class DeferredRenderer: public Renderer {

public:
  void setScene(RenderScene& scene);
  void onLoad(RHIContext& ctx) override;

  void onRenderFrame(RHIContext& ctx) override;

  void onRenderGui(RHIContext& ctx) override {};

protected:
  void cleanBuffers(RHIContext& ctx);
  void generateGbuffer(RHIContext& ctx);
  void deferredShading(RHIContext& ctx);
  void copyToBackBuffer(RHIContext& ctx);

  // G-buffers
  RenderScene* mTargetScene = nullptr;
  Texture2::sptr_t mGAlbedo;
  Texture2::sptr_t mGDepth;
  Texture2::sptr_t mGPosition;
  Texture2::sptr_t mGNormal;

  Texture2::sptr_t mTFinal;
};

//
// class DeferredRenderer {
// public:
//   void bind(const RenderScene& scene);
// protected:
//   RenderGraph mRenderGraph;
//   Texture2::sptr_t mFinalTexture;
// };

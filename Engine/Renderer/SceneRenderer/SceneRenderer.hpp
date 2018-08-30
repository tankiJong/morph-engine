#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/RHI/FrameBuffer.hpp"
#include "Engine/Graphics/RHI/TypedBuffer.hpp"

// deferred renderer
/*
* Shader binding:
*
* (Reference to eUniformSlot)
*
* The whole Root Signature layouted like this:
[0]Descriptor Set - Per Frame Data - Update in Begin Frame
b0: UNIFORM_TIME,

[0]Descriptor Set - Per View Data - Update when switch camera
b1: UNIFORM_CAMERA,
b2: UNIFORM_TRANSFORM,
b3: UNIFORM_LIGHT,
t0: TEXTURE_AO

[1]Descripotr Set - Per Instance Data - update on each draw call, live on [[Material]]
t1: TEXTURE_DIFFUSE,
t2: TEXTURE_NORMAL,
t3: TEXTURE_SPECULAR,
*
*
*
*/
class SceneRenderer : public Renderer {
public:
  SceneRenderer(const RenderScene& target);

  void onLoad(RHIContext& ctx) override;

  void onRenderFrame(RHIContext& ctx) override;

  // void onRenderGui(RHIContext& ctx) override;

protected:
  void genGBuffer(RHIContext& ctx);
  void genAO(RHIContext& ctx);
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

  S<DescriptorSet> mDescriptorSet;

};

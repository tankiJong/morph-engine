#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/RHI/FrameBuffer.hpp"
#include "Engine/Graphics/RHI/TypedBuffer.hpp"
#include <fstream>

// deferred renderer
/*
* Shader binding:
*
* (Reference to eUniformSlot)
*
*
* The whole Root Signature layouted like this:
[0] Renderer Descriptor Set - Renderer managed data
b0: UNIFORM_FRAME,    - Update per frame
b1: UNIFORM_CAMERA,   - Update per view
b2: UNIFORM_TRANSFORM,- Update per instance
b3: UNIFORM_LIGHT,    - Update per frame
t0: TEXTURE_AO        - Update per frame
s0: static sampler

[1]Descripotr Set - Per Instance Data - update on each draw call, live on [[Material]]
b6: UNIFORM_USER_1
b7: UNIFORM_USER_2
b8: UNIFORM_USER_3
b9: UNIFORM_USER_4
b10: UNIFORM_USER_5
t1: TEXTURE_DIFFUSE,
t2: TEXTURE_NORMAL,
t3: TEXTURE_SPECULAR,
t4: TEXTURE_USER_1
t5: TEXTURE_USER_2

[2] G-Buffer Data
t10: G_Albedo
t11: G_Normal
t12: G_Position
t13: G_Depth


==[Gen G-Buffer]
RootSignature: 
0 - [0]
1 - [1]

==[Gen AO]==
0 - [0]
1 - [2], t14 - AccelerationStructure
3 - UAV(0 - AO)

*
*
*
*/
class SceneRenderer : public Renderer {
public:
  SceneRenderer(const RenderScene& target);
  ~SceneRenderer();
  void onLoad(RHIContext& ctx) override;

  void onRenderFrame(RHIContext& ctx) override;

  // void onRenderGui(RHIContext& ctx) override;

protected:
  void updateDescriptors();
  void genGBuffer(RHIContext& ctx);
  void genAO(RHIContext& ctx);
  void computeSurfelCoverage(RHIContext& ctx);
  void accumlateSurfels(RHIContext& ctx);
  void accumlateGI(RHIContext& ctx);
  void visualizeSurfels(RHIContext& ctx);
  void deferredLighting(RHIContext& ctx);
  void computeIndirectLighting(RHIContext& ctx);
  void setupFrame();
  void setupView(RHIContext& ctx);
  void dumpSurfels(RHIContext& ctx);
  void pathTracing(RHIContext& ctx);
  void fxaa(RHIContext& ctx);
  const RenderScene& mTargetScene;

  bool shouldRecomputeIndirect() const;
  FrameBuffer mGFbo;
  // G-Buffers
  Texture2::sptr_t mGAlbedo;
  Texture2::sptr_t mGNormal;
  Texture2::sptr_t mGPosition;
  Texture2::sptr_t mGDepth;
  Texture2::sptr_t mGVelocity;
  Texture2::sptr_t mGAO;

  Texture2::sptr_t mIndirectLight;

  Texture2::sptr_t mAO;
  Texture2::sptr_t mScene;
  Texture2::sptr_t mSurfelVisual;
  Texture2::sptr_t mSurfelCoverage;
  Texture2::sptr_t mSurfelSpawnChance;
  Texture2::sptr_t mFinalColor;

  // cbo
  frame_data_t mFrameData;
  S<RHIBuffer> mcFrameData;
  S<RHIBuffer> mcCamera;
  S<RHIBuffer> mcModel;
  S<RHIBuffer> mcLight;
  TypedBuffer::sptr_t mAccelerationStructure;
  TypedBuffer::sptr_t mSurfelsBuffer[2];
  TypedBuffer::sptr_t mSurfels[2];
  TypedBuffer::sptr_t mSurfelsHistory;
  TypedBuffer::sptr_t mSurfelBuckets;

  S<DescriptorSet> mDSharedDescriptors;
  S<DescriptorSet> mDGBufferDescriptors;
  S<DescriptorSet> mDGenAOUavDescriptors;
  S<DescriptorSet> mDAccumulateSurfelUavDescriptors;
  S<DescriptorSet> mDSurfelVisualDescriptors;
  S<DescriptorSet> mDSurfelGIDescriptors;
  S<DescriptorSet> mDDeferredLightingDescriptors;
  S<DescriptorSet> mDDeferredLightingIndirectDescriptors;

  std::ofstream mSurfelDump;

  camera_t mCameraData[2];
};

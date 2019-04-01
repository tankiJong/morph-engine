#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIType.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"

class Mesh;
class Material;
class FrameBuffer;
class Program;
class Camera;
class FrameBuffer;
class RootSignature;
class GraphicsState;
class DepthStencilView;
class RenderTargetView;
class ShaderResourceView;
class ConstantBufferView;
struct vertex_lit_t;
class DescriptorSet;
class RHIContext;

class ImmediateRenderer {
public:
  void startUp();
  void cleanUp();

  const FrameBuffer* currentFbo() const { return mFrameBuffer.get(); }

  S<RHIContext>& context() { return mRhiContext; };
  FrameBuffer* currentFbo() { return mFrameBuffer.get(); };

  void drawSubMesh(Mesh& mesh, uint subMeshIndex);
  void drawMesh(Mesh& mesh);
  void drawMeshImmediate(span<const vertex_lit_t> vertices, eDrawPrimitive prim);

  void setDepthStencilTarget(const DepthStencilView* dsv);
  void setLight(uint index, const light_info_t& lightInfo);
  void setMaterial(Material& material);
  void setModelMatrix(const mat44& model);
  void setProgram(S<const Program>& program);
  void setRenderTarget(const RenderTargetView* rtv, uint index = 0);
  void setTexture(eTextureSlot slot, const ShaderResourceView& srv);
  void setUniform(eUniformSlot slot, const ConstantBufferView& cbv);
  void setView(const Camera& cam);
  void setRenderRegion(const RHITexture& rt, const vec2& offsetBegin = vec2::zero, const vec2& offsetEnd = vec2::zero);
  const DepthStencilView* defaultDsv() const;
  const RenderTargetView& defaultRtv() const;

  static ImmediateRenderer& get();

protected:
  S<FrameBuffer>    mFrameBuffer       = nullptr;
  S<const Program>  mProgram           = nullptr;
  S<GraphicsState>  mPipelineState     = nullptr;
  S<RootSignature>  mRootSignature     = nullptr;
  S<DescriptorSet>  mGpuDescriptorSet  = nullptr;
  S<RHIContext>     mRhiContext        = nullptr;
  S<DescriptorSet>  mDescriptorSet     = nullptr;
  Material*         mMaterial          = nullptr;
  RHIBuffer::sptr_t mLightBuffer       = nullptr;
  RHIBuffer::sptr_t mModelMatrixBuffer = nullptr;
  RHIBuffer::sptr_t mCameraBuffer      = nullptr;
  bool              mIsStateDirty      = true;
};

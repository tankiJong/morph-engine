#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIType.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Engine/Graphics/Model/Vertex.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/Camera.hpp"

class Mesh;
class Material;
class FrameBuffer;
class Program;
class FrameBuffer;
class RootSignature;
class GraphicsState;
class ImmediateRenderer {
public:
  void startUp();
  void cleanUp();

  const FrameBuffer* currentFbo() const { return mFrameBuffer.get(); };
  FrameBuffer* currentFbo() { return mFrameBuffer.get(); };

  void drawMesh(Mesh& mesh);
  void drawMeshImmediate(span<const vertex_lit_t> vertices, eDrawPrimitive prim);

  void setDepthStencilTarget(const DepthStencilView* dsv);
  void setLight(uint index, const light_info_t& lightInfo);
  void setMaterial(Material& material);
  void setModelMatrix(const mat44& model);
  void setProgram(S<Program>& program);
  void setRenderTarget(const RenderTargetView* rtv, uint index = 0);
  void setTexture(eTextureSlot slot, const ShaderResourceView& srv);
  void setUniform(eUniformSlot slot, const ConstantBufferView& cbv);
  void setView(const Camera& cam);
  DepthStencilView& defaultDsv();
  RenderTargetView& defaultRtv();

  S<Program>& program() { return mProgram; }

protected:
  S<FrameBuffer>    mFrameBuffer       = nullptr;
  S<Program>        mProgram           = nullptr;
  S<GraphicsState>  mPipelineState     = nullptr;
  S<RootSignature>  mRootSignature     = nullptr;
  S<DescriptorSet>  mGpuDescriptorSet  = nullptr;
  S<RHIContext>     mRhiContext        = nullptr;
  S<DescriptorSet>  mDescriptorSet     = nullptr;
  Material*         mMaterial          = nullptr;
  RHIBuffer::sptr_t mLightBuffer       = nullptr;
  RHIBuffer::sptr_t mModelMatrixBuffer = nullptr;
  RHIBuffer::sptr_t mCameraBuffer      = nullptr;
};

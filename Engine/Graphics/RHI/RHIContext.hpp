#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIContextData.hpp"
#include "Engine/Graphics/RHI/TypedBuffer.hpp"

class aabb2;
class ComputeState;
class DepthStencilView;
class RenderTargetView;
class FrameBuffer;
class GraphicsState;
class RHIResource;
class RHITexture;
class RHIBuffer;
class RHITexture;
class RootSignature;

enum eTransitionBarrierFlag {
  TRANSITION_FULL = 0,
  TRANSITION_BEGIN = 0x1,
  TRANSITION_END = (0x1 << 1),
};
class RHIContext: std::enable_shared_from_this<RHIContext> {
public:
  using sptr_t = S<RHIContext>;
  virtual RHIContextData::sptr_t contextData() const { return mContextData; }
  // void initDefaultRenderTarget();
  virtual void flush();

  //-------------- resource operations --------------

  void copyBufferRegion(const RHIBuffer* dst, size_t dstOffset, RHIBuffer* src, size_t srcOffset, size_t byteCount);
  void updateBuffer(RHIBuffer* buffer, const void* data, size_t offset = 0, size_t byteCount = 0);

  void updateTextureSubresources(const RHITexture& texture, uint firstSubresource, uint subresourceCount, const void* data);
  void updateTexture(const RHITexture& texture, const void* data);

  void copyResource(const RHIResource& from, RHIResource& to);
  void copySubresource(const RHITexture& from, uint fromSubIndex, const RHITexture& to, uint toSubIndex);
  size_t readBuffer(const RHIBuffer& res, void* data, size_t maxSize);
  virtual void transitionBarrier(const RHIResource* res, RHIResource::State newState, eTransitionBarrierFlag = TRANSITION_FULL, const ResourceViewInfo* viewInfo = nullptr);
  virtual void uavBarrier(const RHIResource* res);

  //-------------------------------------------------
  // void beforeFrame();
  // void  afterFrame();

  void dispatch(uint threadGroupX, uint threadGroupY, uint threadGroupZ);

  void draw(uint start, uint count);
  void drawIndexed(uint vertStart, uint idxStart, uint count);
  void drawInstanced(uint startVert, uint startIns, uint vertCount, uint insCount);
  void blit(const ShaderResourceView& from, const RenderTargetView& to);
  void setGraphicsRootSignature(const RootSignature& rootSig);
  void setComputeRootSignature(const RootSignature& rootSig);
  void setGraphicsState(const GraphicsState& pso);
  void setComputeState(const ComputeState& pso);
  void setFrameBuffer(const FrameBuffer& fbo);
  void setVertexBuffer(const VertexBuffer& vbo, uint streamIndex);
  void setIndexBuffer(const IndexBuffer* ibo);
  void setPrimitiveTopology(const eDrawPrimitive prim);
  void setViewport(const aabb2& bounds);
  void setScissorRect(const aabb2& rect);
  void bindDescriptorHeap();

  void clearRenderTarget(const RenderTargetView& rtv, const Rgba& rgba);
  void clearRenderTarget(const RenderTargetView& rtv, const vec4& rgba);
  void clearDepthStencilTarget(const DepthStencilView& dsv, 
                               bool clearDepth = true, bool clearStencil = true,
                               float depth = 1.f, u8 stencil = 0);

  void beginEvent(const char* name);
  void endEvent();
  static sptr_t create(command_queue_handle_t commandQueue);

protected:
  void textureBarrier(const RHITexture& tex, RHIResource::State newState, eTransitionBarrierFlag flag);
  void bufferBarrier(const RHIBuffer& buffer, RHIResource::State newState, eTransitionBarrierFlag flag);
  void subresourceBarrier(const RHITexture& tex, RHIResource::State newState, const ResourceViewInfo* viewInfo, eTransitionBarrierFlag flag);

  friend class RHIDevice;
  RHIContextData::sptr_t mContextData;
  bool mCommandsPending = false;
};

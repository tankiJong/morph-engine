#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIContextData.hpp"
#include "Engine/Graphics/RHI/DescriptorPool.hpp"
#include "Game/dx12/Dx12Device.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"
#include "Engine/Graphics/RHI/PipelineState.hpp"

class RHITexture;

class RHIContext: std::enable_shared_from_this<RHIContext> {
public:
  using sptr_t = S<RHIContext>;
  virtual RHIContextData::sptr_t contextData() const { return mContextData; }
  void initDefaultRenderTarget();
  virtual void flush();

  //-------------- resource operations --------------

  void copyBufferRegion(const RHIBuffer* dst, size_t dstOffset, RHIBuffer* src, size_t srcOffset, size_t byteCount);

  void updateBuffer(const RHIBuffer* buffer, const void* data, size_t offset = 0, size_t byteCount = 0);

  void updateTextureSubresources(const RHITexture& texture, uint firstSubresource, uint subresourceCount, const void* data);

  void updateTexture(const RHITexture& texture, const void* data);
  virtual void resourceBarrier(const RHIResource* res, RHIResource::State newState);

  //-------------------------------------------------
  void beforeFrame();
  void afterFrame();
  void draw(uint start, uint count);
  void drawInstanced(uint startVert, uint startIns, uint vertCount, uint insCount);
  void setPipelineState(const PipelineState::sptr_t& pso);
  void setVertexBuffer(const S<RHIBuffer>& vbo, uint elementSize, uint streamIndex);
  void bindDescriptorHeap();
  static sptr_t create(command_queue_handle_t commandQueue);

protected:
  friend class RHIDevice;
  RHIContextData::sptr_t mContextData;
  bool mCommandsPending = false;
};

#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/RHI/ConstantBuffer.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

ConstantBufferView::sptr_t ConstantBufferView::create(W<RHIBuffer> res) {
  RHIBuffer::scptr_t ptr = res.lock();

  if(!ptr && sNullView) {
    return sNullView;
  }

  D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
  RHIBuffer::handle_t resHandle = nullptr;

  if(ptr) {
    desc.BufferLocation = ptr->gpuAddress();
    desc.SizeInBytes = (uint)ptr->size();
    resHandle = ptr->handle();
  }

  DescriptorSet::Layout layout;
  layout.addRange(DescriptorSet::Type::Cbv, 0, 1);
  rhi_handle_t handle = DescriptorSet::create(RHIDevice::get()->cpuDescriptorPool(), layout);
  ENSURES(handle);
  RHIDevice::get()->nativeDevice()->CreateConstantBufferView(&desc, handle->cpuHandle(0));

  sptr_t obj = sptr_t();
  sptr_t& cbv = ptr ? obj : sNullView;

  cbv = sptr_t(new ConstantBufferView(res, handle));

  return cbv;
}

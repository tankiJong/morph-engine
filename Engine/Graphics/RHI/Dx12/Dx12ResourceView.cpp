#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/RHI/ConstantBuffer.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"

ShaderResourceView::sptr_t ShaderResourceView::create(
  W<RHIResource> res, uint mostDetailedMip, uint mipCount, uint firstArraySlice, uint arraySize) {
  
  RHIResource::sptr_t ptr = res.lock();

  if(!ptr && sNullView) {
    return sNullView;
  }

  D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
  RHIResource::handle_t resHandle = nullptr;

  if(ptr) {
    // this is simple hacky version for texture 2d
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipLevels = 1;

  } else {
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  }

  sptr_t obj;
  sptr_t& srv = ptr ? obj : sNullView;

  DescriptorSet::Layout layout;
  layout.addRange(DescriptorSet::Type::TextureSrv, 0, 1);
  rhi_handle_t handle = DescriptorSet::create(RHIDevice::get()->cpuDescriptorPool(), layout);

  ENSURES(handle);

  RHIDevice::get()->nativeDevice()->CreateShaderResourceView(
    ptr ? ptr->handle() : nullptr, &desc, handle->cpuHandle(0));

  srv = sptr_t(new ShaderResourceView(res, handle, mostDetailedMip, mipCount, firstArraySlice, arraySize));
  return srv;
}

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

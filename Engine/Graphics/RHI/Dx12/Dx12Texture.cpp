#include "Engine/Graphics/RHI/RHITexture.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12Resource.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"

bool RHITexture::rhiInit(const void* data, size_t size) {
  D3D12_RESOURCE_DESC desc = {};

  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.Width = mWidth;
  desc.Height = mHeight;
  desc.Flags = asDx12ResourceFlags(mBindingFlags);
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  desc.Alignment = 0;
  desc.DepthOrArraySize = 1;

  d3d_call(RHIDevice::get()->nativeDevice()->CreateCommittedResource(
    &DefaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON,
    nullptr, IID_PPV_ARGS(&mRhiHandle)
  ));
  mRhiHandle->SetName(L"Texture");
  if(data) {
    RHIDevice::get()->defaultRenderContext()->updateTexture(*this, data);
  }

  // automatically create srv for now

  auto pool = RHIDevice::get()->gpuDescriptorPool();

  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;

  RHIDevice::get()->nativeDevice()->CreateShaderResourceView(
    mRhiHandle, &srvDesc, pool->rhiData()->heaps[D3D12_DESCRIPTOR_RANGE_TYPE_SRV]->cpuHandleBase());
}

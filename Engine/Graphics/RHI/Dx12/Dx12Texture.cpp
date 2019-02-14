#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12Resource.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"

bool Texture2::rhiInit(const void* data, size_t /*size*/) {
  TODO("`size` should be used here");
  D3D12_RESOURCE_DESC desc = {};

  desc.MipLevels = 1;
  desc.Format = toDXGIFormat(mFormat);
  desc.Width = mWidth;
  desc.Height = mHeight;
  desc.Flags = asDx12ResourceFlags(mBindingFlags);
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  desc.Alignment = 0;
  desc.DepthOrArraySize = 1;

  D3D12_CLEAR_VALUE clearValue = {};
  D3D12_CLEAR_VALUE* clearValPtr = {};

  if((mBindingFlags & (Texture2::BindingFlag::RenderTarget | Texture2::BindingFlag::DepthStencil)) != Texture2::BindingFlag::None) {

    clearValue.Format = desc.Format;
    if((mBindingFlags & Texture2::BindingFlag::DepthStencil) != Texture2::BindingFlag::None) {
      clearValue.DepthStencil.Depth = 1.f;
    }
    clearValPtr = &clearValue;
  }

  if(mFormat == TEXTURE_FORMAT_D24S8 && is_set(mBindingFlags, Texture2::BindingFlag::ShaderResource | Texture2::BindingFlag::UnorderedAccess)) {
    TODO("set to typeless later when do the unorder access");
    clearValPtr = nullptr;
  }

  d3d_call(RHIDevice::get()->nativeDevice()->CreateCommittedResource(&DefaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, clearValPtr, IID_PPV_ARGS(&mRhiHandle)));

  mRhiHandle->SetName(L"Texture");
  if(data) {
    RHIDevice::get()->defaultRenderContext()->updateTexture(*this, data);
  }

  return true;
}

RHITexture::RHITexture(rhi_resource_handle_t res): RHIResource(res) {
  D3D12_RESOURCE_DESC desc = res->GetDesc();
  mFormat = toTextureFormat(desc.Format);
  mWidth = (uint)desc.Width;
  mHeight = (uint)desc.Height;
  mDepth = (uint)desc.DepthOrArraySize;
}

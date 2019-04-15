#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"


void initTextureSrv(const RHITexture* res, uint mostDetailedMip, uint mipCount, uint firstArraySlice, uint arraySize, D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {

  if(res->format() == TEXTURE_FORMAT_D24S8) {
    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  } else {
    desc.Format = toDXGIFormat(res->format());
  }


  bool isTextureArray = arraySize > 1;

  switch(res->type()) {
    case RHIResource::Type::Texture1D:
      if(isTextureArray) {
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        desc.Texture1DArray.MipLevels = mipCount;
        desc.Texture1DArray.MostDetailedMip = mostDetailedMip;
        desc.Texture1DArray.ArraySize = arraySize;
        desc.Texture1DArray.FirstArraySlice = firstArraySlice;
      } else {
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
        desc.Texture1D.MipLevels = mipCount;
        desc.Texture1D.MostDetailedMip = mostDetailedMip;
      }
    break;
    case RHIResource::Type::Texture2D:
      if(isTextureArray) {
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.MipLevels = mipCount;
        desc.Texture2DArray.MostDetailedMip = mostDetailedMip;
        desc.Texture2DArray.ArraySize = arraySize;
        desc.Texture2DArray.FirstArraySlice = firstArraySlice;
      } else {
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = mipCount;
        desc.Texture2D.MostDetailedMip = mostDetailedMip;
      }
    break;
    case RHIResource::Type::TextureCube:
      if(arraySize > 1) {
        desc.TextureCubeArray.First2DArrayFace = 0;
        desc.TextureCubeArray.NumCubes = arraySize;
        desc.TextureCubeArray.MipLevels = mipCount;
        desc.TextureCubeArray.MostDetailedMip = mostDetailedMip;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
      } else {
        desc.TextureCube.MipLevels = mipCount;
        desc.TextureCube.MostDetailedMip = mostDetailedMip;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
      }
    break;
    case RHIResource::Type::Texture3D:
      desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
      desc.Texture3D.MipLevels = mipCount;
      desc.Texture3D.MostDetailedMip = mostDetailedMip;
    break;
    default: 
    BAD_CODE_PATH();
  }

  desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
}

void initRtv(const RHITexture* res, uint miplevel, uint firstArraySlice, uint arraySize, D3D12_RENDER_TARGET_VIEW_DESC& desc) {
  desc = {};
  uint32_t arrayMultiplier = (res->type() == RHIResource::Type::TextureCube ? 6 : 1);

  if(arraySize == ResourceViewInfo::MAX_POSSIBLE) {
    arraySize = res->arraySize() - firstArraySlice;
  }
  
  switch(res->type()) { 
    case RHIResource::Type::Texture1D:
    if(res->arraySize() > 1) {
      desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
      desc.Texture1DArray.ArraySize = arraySize;
      desc.Texture1DArray.FirstArraySlice = firstArraySlice;
      desc.Texture1DArray.MipSlice = miplevel;
    } else {
      desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
      desc.Texture1D.MipSlice = miplevel;
    }
    break;
    case RHIResource::Type::Texture2D:
    case RHIResource::Type::TextureCube:
    {
      // for now, only support bind face separately
      desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      desc.Texture2DArray.ArraySize = 1;
      desc.Texture2DArray.FirstArraySlice = firstArraySlice;
      desc.Texture2DArray.MipSlice = miplevel;
    }
    break;
    default: 
    BAD_CODE_PATH();
  }
}

ShaderResourceView::sptr_t ShaderResourceView::create(
  W<const RHITexture> res, uint mostDetailedMip, uint mipCount, uint firstArraySlice, uint arraySize) {
  
  RHITexture::scptr_t ptr = res.lock();

  if(!ptr && sNullView) {
    return sNullView;
  }

  D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

  if(ptr) {
    initTextureSrv(ptr.get(), mostDetailedMip, mipCount, firstArraySlice, arraySize, desc);
    // this is simple hacky version for texture 2d
    // desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    // if(ptr->format() == TEXTURE_FORMAT_D24S8) {
    //   desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    // } else {
    //   desc.Format = toDXGIFormat(ptr->format());
    //
    // }
    // desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    // desc.Texture2D.MipLevels = 1;
 
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
    ptr ? ptr->handle().Get() : nullptr, &desc, handle->cpuHandle(0));

  srv = sptr_t(new ShaderResourceView(res, DescriptorPool::Type::TextureSrv, handle, mostDetailedMip, mipCount, firstArraySlice, arraySize));
  return srv;
}

ShaderResourceView::sptr_t ShaderResourceView::create(const TypedBuffer& res) {

  D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
  RHIResource::handle_t resHandle = nullptr;

  desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
  desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.Buffer.FirstElement = 0;
  desc.Buffer.NumElements = res.elementCount();
  desc.Buffer.StructureByteStride = res.stride();

  sptr_t srv;

  DescriptorSet::Layout layout;
  layout.addRange(DescriptorSet::Type::TextureSrv, 0, 1);
  rhi_handle_t handle = DescriptorSet::create(RHIDevice::get()->cpuDescriptorPool(), layout);

  ENSURES(handle);

  RHIDevice::get()->nativeDevice()->CreateShaderResourceView(
    res.handle().Get(), &desc, handle->cpuHandle(0));
  srv = sptr_t(new ShaderResourceView(res.shared_from_this(), DescriptorPool::Type::TypedBufferSrv, handle, 0, ResourceViewInfo::MAX_POSSIBLE, 0, ResourceViewInfo::MAX_POSSIBLE));
  return srv;
}

ConstantBufferView::sptr_t ConstantBufferView::create(W<const RHIBuffer> res) {
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

RenderTargetView::sptr_t RenderTargetView::create(W<const RHITexture> res, uint mipLevel, uint firstArraySlice, uint arraySize) {

  RHITexture::scptr_t ptr = res.lock();

  if(!ptr && sNullView) {
    return sNullView;
  }

  D3D12_RENDER_TARGET_VIEW_DESC desc = {};
  RHIResource::handle_t resHandle = nullptr;

  if(ptr) {
    initRtv(ptr.get(), mipLevel, firstArraySlice, arraySize, desc);
    resHandle = ptr->handle();
  } else {
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
  }

  DescriptorSet::Layout layout;

  layout.addRange(DescriptorSet::Type::Rtv, 0, 1);
  rhi_handle_t handle = DescriptorSet::create(RHIDevice::get()->cpuDescriptorPool(), layout);
  ENSURES(handle);

  RHIDevice::get()->nativeDevice()->CreateRenderTargetView(resHandle.Get(), &desc, handle->cpuHandle(0));

  sptr_t obj;

  sptr_t& rtv = ptr ? obj : sNullView;

  rtv = sptr_t(new RenderTargetView(res, DescriptorPool::Type::Rtv, handle, mipLevel, firstArraySlice, arraySize));

  return rtv;
}

DepthStencilView::sptr_t DepthStencilView::create(W<const RHITexture> res, uint mipLevel, uint firstArraySlice, uint arraySize) {
  
  RHITexture::scptr_t ptr = res.lock();

  if(!ptr && sNullView) {
    return sNullView;
  }

  D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
  RHIResource::handle_t resHandle = nullptr;

  if(ptr) {
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = mipLevel;
    resHandle = ptr->handle();
  } else {
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  }

  DescriptorSet::Layout layout;

  layout.addRange(DescriptorSet::Type::Dsv, 0, 1);
  rhi_handle_t handle = DescriptorSet::create(RHIDevice::get()->cpuDescriptorPool(), layout);
  ENSURES(handle);

  RHIDevice::get()->nativeDevice()->CreateDepthStencilView(resHandle.Get(), &desc, handle->cpuHandle(0));

  sptr_t obj;

  sptr_t& dsv = ptr ? obj : sNullView;

  dsv = sptr_t(new DepthStencilView(res, DescriptorPool::Type::Dsv, handle, mipLevel, firstArraySlice, arraySize));

  return dsv;
}


UnorderedAccessView::sptr_t UnorderedAccessView::create(const TypedBuffer& res) {
  D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};

  RHIResource::handle_t resHandle = res.handle();
  RHIResource::handle_t counterHandle = res.uavCounter()->handle();

  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
  desc.Buffer.FirstElement = 0;
  desc.Buffer.NumElements = res.elementCount();
  desc.Buffer.StructureByteStride = res.stride();

  // https://docs.microsoft.com/en-us/windows/desktop/direct3d12/uav-counters

  DescriptorSet::Layout layout;
  layout.addRange(DescriptorSet::Type::TextureUav, 0, 1);
  rhi_handle_t handle = DescriptorSet::create(RHIDevice::get()->cpuDescriptorPool(), layout);
  ENSURES(handle);

  RHIDevice::get()->nativeDevice()->CreateUnorderedAccessView(resHandle.Get(), counterHandle.Get(), &desc, handle->cpuHandle(0));

  sptr_t uav = sptr_t(new UnorderedAccessView(res.shared_from_this(), DescriptorPool::Type::TypedBufferUav, handle, 0, 0, 1));

  return uav;
}


UnorderedAccessView::sptr_t UnorderedAccessView::create(W<const RHIBuffer> res) {
  RHIBuffer::scptr_t ptr = res.lock();

  if (!ptr && sNullView) {
    return sNullView;
  }

  D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
  desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

  RHIResource::handle_t resHandle = nullptr;
  RHIResource::handle_t counterHandle = nullptr;

  if (ptr) {
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    desc.Buffer.NumElements = (uint)ptr->size() / sizeof(float);
    resHandle = ptr->handle();
  } else {
    desc.Format = DXGI_FORMAT_UNKNOWN;
  }

  DescriptorSet::Layout layout;
  layout.addRange(DescriptorSet::Type::TextureUav, 0, 1);
  rhi_handle_t handle = DescriptorSet::create(RHIDevice::get()->cpuDescriptorPool(), layout);
  ENSURES(handle);

  RHIDevice::get()->nativeDevice()->CreateUnorderedAccessView(resHandle.Get(), counterHandle.Get(), &desc, handle->cpuHandle(0));

  sptr_t obj;
  sptr_t uav = ptr ? obj : sNullView;
  uav = sptr_t(new UnorderedAccessView(res, DescriptorPool::Type::StructuredBufferUav, handle, 0, 0, 1));

  return uav;
}
UnorderedAccessView::sptr_t UnorderedAccessView::create(W<const RHITexture> res, uint32_t mipLevel, uint32_t firstArraySlice, uint32_t arraySize) {
  RHITexture::scptr_t ptr = res.lock();

  if(!ptr && sNullView) {
    return sNullView;
  }

  D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};

   
  RHIResource::handle_t resHandle = nullptr;
  RHIResource::handle_t counterHandle = nullptr;

  if(ptr) {
    desc.Format = toDXGIFormat(ptr->format());
    resHandle = ptr->handle();
    switch(ptr->type()) { 
      case RHIResource::Type::Texture2D:
        if(arraySize > 1) {
          desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
          desc.Texture2DArray.MipSlice = mipLevel;
          desc.Texture2DArray.ArraySize = arraySize;
          desc.Texture2DArray.FirstArraySlice = firstArraySlice;
          desc.Texture2DArray.PlaneSlice = 0;
        } else {
          desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
          desc.Texture2D.MipSlice = mipLevel;
          desc.Texture2D.PlaneSlice = 0;
        }
      break;
      case RHIResource::Type::Texture3D: 
        desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        desc.Texture3D.MipSlice = mipLevel;
        desc.Texture3D.FirstWSlice = firstArraySlice;
        desc.Texture3D.WSize = arraySize;
      break;
      case RHIResource::Type::TextureCube: 
      case RHIResource::Type::Texture1D: 
      case RHIResource::Type::Buffer: 
      case RHIResource::Type::Unknown: 
      case RHIResource::Type::Texture2DMultisample: 
      default:
      BAD_CODE_PATH();
    }
  } else {
    desc = {};
    desc.Format = DXGI_FORMAT_R32_UINT;
    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
  }


  DescriptorSet::Layout layout;
  layout.addRange(DescriptorSet::Type::TextureUav, 0, 1);
  rhi_handle_t handle = DescriptorSet::create(RHIDevice::get()->cpuDescriptorPool(), layout);
  ENSURES(handle);

  RHIDevice::get()->nativeDevice()->CreateUnorderedAccessView(resHandle.Get(), counterHandle.Get(), &desc, handle->cpuHandle(0));

  sptr_t obj;
  sptr_t uav = ptr ? obj : sNullView;
  uav = sptr_t(new UnorderedAccessView(res, DescriptorPool::Type::TextureUav, handle, mipLevel, firstArraySlice, arraySize));

  return uav;
}

#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12DescriptorData.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12Resource.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/RHI/TiledTexture2.hpp"

RHIHeap::eUsage getUsage(RHIResource::BindingFlag flags) {
  return is_set(flags, RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::DepthStencil) ?
         RHIHeap::USAGE_TEXTURE_RT_DS :
         RHIHeap::USAGE_TEXTURE_NON_RT_DS;
}

bool Texture2::rhiInit(bool genMipmap, const void* data, size_t /*size*/, bool implicitHeap) {
  TODO("`size` should be used here");
  D3D12_RESOURCE_DESC desc = {};

  if(genMipmap) {
    uint dim = mWidth | mHeight | mDepthOrArraySize;
    unsigned long len;
    _BitScanReverse(&len, dim);
    mMipLevels = len + 1u;
  } else {
    mMipLevels = 1;
  }

  desc.MipLevels = (uint16_t)mMipLevels;
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
  D3D12_CLEAR_VALUE* clearValPtr = nullptr;

  if((mBindingFlags & (Texture2::BindingFlag::RenderTarget | Texture2::BindingFlag::DepthStencil)) != Texture2::BindingFlag::None) {

    clearValue.Format = desc.Format;
    if((mBindingFlags & Texture2::BindingFlag::DepthStencil) != Texture2::BindingFlag::None) {
      clearValue.DepthStencil.Depth = 1.f;
    }
    clearValPtr = &clearValue;
  }

  if(mFormat == TEXTURE_FORMAT_D24S8 && is_set(mBindingFlags, Texture2::BindingFlag::ShaderResource | Texture2::BindingFlag::UnorderedAccess)) {
    TODO("set to typeless later when do the unorder access");
    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    clearValPtr = nullptr;
  }

  if(implicitHeap) {
    d3d_call(RHIDevice::get()->nativeDevice()->CreateCommittedResource(&DefaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, clearValPtr, IID_PPV_ARGS(&mRhiHandle)));
  } else {
    auto device = RHIDevice::get()->nativeDevice();
    auto allocInfo = device->GetResourceAllocationInfo(0, 1, &desc);
    mGpuMemory = RHIHeap::create(allocInfo.SizeInBytes, DefaultHeapProps, getUsage(mBindingFlags), allocInfo.Alignment);
    d3d_call(device->CreatePlacedResource(mGpuMemory->handle().Get(), 0, &desc, D3D12_RESOURCE_STATE_COMMON, clearValPtr, IID_PPV_ARGS(&mRhiHandle)));
  }

  mState.subresourceState.resize(mMipLevels * mDepthOrArraySize, mState.globalState);
  mState.subresourceInTransition.resize(mMipLevels * mDepthOrArraySize, mState.globalInTransition);

  mRhiHandle->SetName(L"Texture");
  if(data) {
    RHIDevice::get()->defaultRenderContext()->updateTexture(*this, data);
  }

  if(genMipmap) {
    generateMipmap(*RHIDevice::get()->defaultRenderContext());
  }

  return true;
}

bool TextureCube::rhiInit(bool genMipmap, const void* data, size_t /*size*/, bool implicitHeap) {
  TODO("`size` should be used here");
  D3D12_RESOURCE_DESC desc = {};

  if(genMipmap) {
    uint dim = mWidth | mHeight | mDepthOrArraySize;
    unsigned long len;
    _BitScanReverse(&len, dim);
    mMipLevels = len + 1u;
  } else {
    mMipLevels = 1;
  }

  desc.MipLevels = (uint16_t)mMipLevels;
  desc.Format = toDXGIFormat(mFormat);
  desc.Width = mWidth;
  desc.Height = mHeight;
  desc.Flags = asDx12ResourceFlags(mBindingFlags);
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  desc.Alignment = 0;
  desc.DepthOrArraySize = mDepthOrArraySize; // array size should be one here, hence 6(faces)

  
  D3D12_CLEAR_VALUE clearValue = {};
  D3D12_CLEAR_VALUE* clearValPtr = nullptr;

  if((mBindingFlags & (Texture2::BindingFlag::RenderTarget | Texture2::BindingFlag::DepthStencil)) != Texture2::BindingFlag::None) {

    clearValue.Format = desc.Format;
    if((mBindingFlags & Texture2::BindingFlag::DepthStencil) != Texture2::BindingFlag::None) {
      clearValue.DepthStencil.Depth = 1.f;
    }
    clearValPtr = &clearValue;
  }

  if(mFormat == TEXTURE_FORMAT_D24S8 && is_set(mBindingFlags, Texture2::BindingFlag::ShaderResource | Texture2::BindingFlag::UnorderedAccess)) {
    TODO("set to typeless later when do the unorder access");
    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    clearValPtr = nullptr;
  }

  if(implicitHeap) {
    d3d_call(RHIDevice::get()->nativeDevice()->CreateCommittedResource(&DefaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, clearValPtr, IID_PPV_ARGS(&mRhiHandle)));
  } else {
    auto device = RHIDevice::get()->nativeDevice();
    auto allocInfo = device->GetResourceAllocationInfo(0, 1, &desc);
    mGpuMemory = RHIHeap::create(allocInfo.SizeInBytes, DefaultHeapProps, getUsage(mBindingFlags), allocInfo.Alignment);
    d3d_call(device->CreatePlacedResource(mGpuMemory->handle().Get(), 0, &desc, D3D12_RESOURCE_STATE_COMMON, clearValPtr, IID_PPV_ARGS(&mRhiHandle)));
  }

  mState.subresourceState.resize(mMipLevels * mDepthOrArraySize, mState.globalState);
  mState.subresourceInTransition.resize(mMipLevels * mDepthOrArraySize, mState.globalInTransition);

  mRhiHandle->SetName(L"Texture");
  if(data) {
    RHIDevice::get()->defaultRenderContext()->updateTexture(*this, data);
  }

  if(genMipmap) {
    generateMipmap(*RHIDevice::get()->defaultRenderContext());
  }

  return true;
}


bool Texture3::rhiInit(bool genMipmap, const void* data, size_t /*size*/, bool implicitHeap) {
  D3D12_RESOURCE_DESC desc = {};
  if(genMipmap) {
    uint dim = mWidth | mHeight | mDepthOrArraySize;
    unsigned long len;
    _BitScanReverse(&len, dim);
    mMipLevels = len + 1u;
  } else {
    mMipLevels = 1;
  }

  desc.MipLevels = (uint16_t)mMipLevels;
  desc.Format = toDXGIFormat(mFormat);
  desc.Width = mWidth;
  desc.Height = mHeight;
  desc.DepthOrArraySize = (UINT16)mDepthOrArraySize;
  desc.Flags = asDx12ResourceFlags(mBindingFlags);
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
  desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  desc.Alignment = 0;

  
  D3D12_CLEAR_VALUE clearValue = {};
  D3D12_CLEAR_VALUE* clearValPtr = nullptr;

  if((mBindingFlags & (Texture2::BindingFlag::RenderTarget | Texture2::BindingFlag::DepthStencil)) != Texture2::BindingFlag::None) {

    clearValue.Format = desc.Format;
    if((mBindingFlags & Texture2::BindingFlag::DepthStencil) != Texture2::BindingFlag::None) {
      clearValue.DepthStencil.Depth = 1.f;
    }
    clearValPtr = &clearValue;
  }

  if(mFormat == TEXTURE_FORMAT_D24S8 && is_set(mBindingFlags, Texture2::BindingFlag::ShaderResource | Texture2::BindingFlag::UnorderedAccess)) {
    TODO("set to typeless later when do the unorder access");
    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    clearValPtr = nullptr;
  }

  if(implicitHeap) {
    d3d_call(RHIDevice::get()->nativeDevice()->CreateCommittedResource(&DefaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, clearValPtr, IID_PPV_ARGS(&mRhiHandle)));
  } else {
    auto device = RHIDevice::get()->nativeDevice();
    auto allocInfo = device->GetResourceAllocationInfo(0, 1, &desc);
    mGpuMemory = RHIHeap::create(allocInfo.SizeInBytes, DefaultHeapProps, getUsage(mBindingFlags), allocInfo.Alignment);
    d3d_call(device->CreatePlacedResource(mGpuMemory->handle().Get(), 0, &desc, D3D12_RESOURCE_STATE_COMMON, clearValPtr, IID_PPV_ARGS(&mRhiHandle)));
  }

  mState.subresourceState.resize(mMipLevels * mDepthOrArraySize, mState.globalState);
  mState.subresourceInTransition.resize(mMipLevels * mDepthOrArraySize, mState.globalInTransition);
  mRhiHandle->SetName(L"Texture");

  if(data) {
    RHIDevice::get()->defaultRenderContext()->updateTexture(*this, data);
  }

  //if(genMipmap) {
  //  generateMipmap(*RHIDevice::get()->defaultRenderContext());
  //}

  return true;


}

RHITexture::RHITexture(rhi_resource_handle_t res): RHIResource(res) {
  D3D12_RESOURCE_DESC desc = res->GetDesc();
  mFormat = toTextureFormat(desc.Format);
  mWidth = (uint)desc.Width;
  mHeight = (uint)desc.Height;
  mMipLevels = (uint)desc.MipLevels;
  mDepthOrArraySize = (uint)desc.DepthOrArraySize;
  mState.global = true;
  mState.globalState = State::Undefined;
}

bool TiledTexture2::rhiInit(bool genMipmap, const void*, size_t, bool implicitHeap) {
TODO("`size` should be used here");
  D3D12_RESOURCE_DESC desc = {};

  if(genMipmap) {
    uint dim = mWidth | mHeight | mDepthOrArraySize;
    unsigned long len;
    _BitScanReverse(&len, dim);
    mMipLevels = len + 1u;
  } else {
    mMipLevels = 1;
  }

  desc.MipLevels = (uint16_t)mMipLevels;
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
  D3D12_CLEAR_VALUE* clearValPtr = nullptr;

  if((mBindingFlags & (Texture2::BindingFlag::RenderTarget | Texture2::BindingFlag::DepthStencil)) != Texture2::BindingFlag::None) {

    clearValue.Format = desc.Format;
    if((mBindingFlags & Texture2::BindingFlag::DepthStencil) != Texture2::BindingFlag::None) {
      clearValue.DepthStencil.Depth = 1.f;
    }
    clearValPtr = &clearValue;
  }

  if(mFormat == TEXTURE_FORMAT_D24S8 && is_set(mBindingFlags, Texture2::BindingFlag::ShaderResource | Texture2::BindingFlag::UnorderedAccess)) {
    TODO("set to typeless later when do the unorder access");
    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    clearValPtr = nullptr;
  }


  auto device = RHIDevice::get()->nativeDevice();
  d3d_call(device->CreateReservedResource(&desc, D3D12_RESOURCE_STATE_COMMON, clearValPtr, IID_PPV_ARGS(&mRhiHandle)));


  mState.subresourceState.resize(mMipLevels * mDepthOrArraySize, mState.globalState);
  mState.subresourceInTransition.resize(mMipLevels * mDepthOrArraySize, mState.globalInTransition);

  mRhiHandle->SetName(L"Tiled Texture2");

  //if(genMipmap) {
  //  generateMipmap(*RHIDevice::get()->defaultRenderContext());
  //}

  return true;
}

void TiledTexture2::tile(uint x, uint y, Texture2::sptr_t& tex) {
	EXPECTS(tex->width() == mTileWidth && tex->height() == mTileHeight);
  EXPECTS(tex->mipCount() == mipCount());
	tile(x, y) = tex;

  uint tileMapCount = mipCount();

  D3D12_TILED_RESOURCE_COORDINATE* tileCoords 
    = (D3D12_TILED_RESOURCE_COORDINATE*)_alloca(sizeof(D3D12_TILED_RESOURCE_COORDINATE) * tileMapCount);
  D3D12_TILE_REGION_SIZE* tileRegionSize
    = (D3D12_TILE_REGION_SIZE*)_alloca(sizeof(D3D12_TILE_REGION_SIZE) * tileMapCount);
  for(uint i = 0; i < tileMapCount; i++) {
    tileCoords[i].X = 0;
    tileCoords[i].Y = 0;
    tileCoords[i].Z = 0;
    tileCoords[i].Subresource = i;
    //tileRegionSize[i].
  }
	//RHIDevice::get()->defaultRenderContext()->contextData()
	//								->commandQueue()
	//								->UpdateTileMappings(mRhiHandle.Get(), 1, )
}

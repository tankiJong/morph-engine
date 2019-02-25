#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/RHI/RHIType.hpp"
#include "Engine/Math/Primitives/uvec2.hpp"
#include <unordered_map>

class ShaderResourceView;

#undef min
#undef max
class RHITexture: public RHIResource, public inherit_shared_from_this<RHIResource, RHITexture> {
public:
  using sptr_t = S<RHITexture>;
  using scptr_t = S<const RHITexture>;
  using inherit_shared_from_this<RHIResource, RHITexture>::shared_from_this;

  void generateMipmap(RHIContext& ctx);

  uint width(uint mipLevel = 0) const {
    return (mipLevel < mMipLevels) ? std::max(1u, mWidth >> mipLevel) : 0u;
  }
  uint height(uint mipLevel = 0) const {
    return (mipLevel < mMipLevels) ? std::max(1u, mHeight >> mipLevel) : 0u;
  }
  
  uint arraySize() const { return mArraySize; }
  uint mipCount() const { return mMipLevels; }
  uvec2 size(uint mipLevel= 0) const { return uvec2{ width(mipLevel), height(mipLevel)}; }
  eTextureFormat format() const { return mFormat; }
  uint subresourceIndex(uint arraySlice, uint mipLevel) const {
    return mipLevel + arraySlice * mMipLevels;
  }
  State subresourceState(uint arraySlice, uint mip) const {
    uint subIndex = subresourceIndex(arraySlice, mip);
    return mState.global ? mState.globalState : mState.subresourceState[subIndex];
  }

  virtual ShaderResourceView* srv(uint mipLevel = 0, uint mipCount = ResourceViewInfo::MAX_POSSIBLE) const override;
  virtual const RenderTargetView* rtv(uint mipLevel = 0, uint arraySlice = 0) const override;
  virtual const DepthStencilView* dsv(uint mipLevel = 0) const override;
  virtual const UnorderedAccessView* uav(uint mipLevel = 0) const override;

  void setSubresourceState(uint arraySlice, uint32_t mipLevel, State newState) const;
  void markSubresourceInTransition(uint arraySlice, uint32_t mipLevel, bool inTransition) const;

  void invalidateViews();
  virtual ~RHITexture() = default;


protected:
  RHITexture(RHIResource::Type type, uint width, uint height, 
             uint depth, uint arraySize, eTextureFormat format, BindingFlag flags, const void* /*data*/, size_t /*size*/)
    : RHIResource(type, flags), mWidth(width), mHeight(height), mDepth(depth), mArraySize(arraySize), mFormat(format) {
  }
  RHITexture(rhi_resource_handle_t res);
  virtual bool rhiInit(bool genMipmap, const void* data, size_t size) = 0;

  uint mWidth = 0;
  uint mHeight = 0;
  uint mDepth = 0;
  uint mArraySize = 0;
  uint mMipLevels = 0;

  mutable std::unordered_map<ResourceViewInfo, ShaderResourceView::sptr_t>  mSrvs;
  mutable std::unordered_map<ResourceViewInfo, RenderTargetView::sptr_t>    mRtvs;
  mutable std::unordered_map<ResourceViewInfo, DepthStencilView::sptr_t>    mDsvs;
  mutable std::unordered_map<ResourceViewInfo, UnorderedAccessView::sptr_t> mUavs;

  eTextureFormat mFormat = TEXTURE_FORMAT_RGBA8;
};



inline void RHITexture::setSubresourceState(uint arraySlice, uint32_t mipLevel, State newState) const {
  if(mState.global) {
    std::fill(mState.subresourceState.begin(), mState.subresourceState.end(), mState.globalState);
  }
  mState.global = false;
  uint index = subresourceIndex(arraySlice, mipLevel);
  mState.subresourceState[index] = newState;
}

inline void RHITexture::markSubresourceInTransition(uint arraySlice, uint32_t mipLevel, bool inTransition) const {
  EXPECTS(!mState.global);
  mState.subresourceInTransition[subresourceIndex(arraySlice, mipLevel)] = inTransition;
}
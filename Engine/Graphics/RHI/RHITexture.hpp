#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/RHI/RHIType.hpp"
#include "Engine/Math/Primitives/uvec2.hpp"

class ShaderResourceView;

#undef min
#undef max
class RHITexture: public RHIResource, public inherit_shared_from_this<RHIResource, RHITexture> {
public:
  using sptr_t = S<RHITexture>;
  using scptr_t = S<const RHITexture>;
  using inherit_shared_from_this<RHIResource, RHITexture>::shared_from_this;

  uint width(uint mipLevel = 0) const {
    return (mipLevel < mMipLevels) ? std::max(1u, mWidth >> mipLevel) : 0u;
  }
  uint height(uint mipLevel = 0) const {
    return (mipLevel < mMipLevels) ? std::max(1u, mHeight >> mipLevel) : 0u;
  }
  inline uint arraySize() const { return mArraySize; }
  uvec2 size() const { return uvec2{ mWidth, mHeight }; }
  eTextureFormat format() const { return mFormat; }
  virtual ShaderResourceView* srv(uint mipLevel = 0) const override;
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
  mutable ShaderResourceView::sptr_t mSrv[13];

  eTextureFormat mFormat = TEXTURE_FORMAT_RGBA8;
};




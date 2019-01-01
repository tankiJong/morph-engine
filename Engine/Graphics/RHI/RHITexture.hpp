﻿#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/RHI/RHIType.hpp"
#include "Engine/Math/Primitives/uvec2.hpp"

class ShaderResourceView;

class RHITexture: public RHIResource, public inherit_shared_from_this<RHIResource, RHITexture> {
public:
  using sptr_t = S<RHITexture>;
  using scptr_t = S<const RHITexture>;
  using inherit_shared_from_this<RHIResource, RHITexture>::shared_from_this;

  uint width() const { return mWidth; }
  uint height() const { return mHeight; }
  uvec2 size() const { return uvec2{ mWidth, mHeight }; }
  eTextureFormat format() const { return mFormat; }
  const ShaderResourceView& srv() const;
protected:
  RHITexture(RHIResource::Type type, uint width, uint height, 
             uint depth, eTextureFormat format, BindingFlag flags, const void* /*data*/, size_t /*size*/)
    : RHIResource(type, flags), mWidth(width), mHeight(height), mDepth(depth), mFormat(format) {
  }
  RHITexture(rhi_resource_handle_t res);
  virtual bool rhiInit(const void* data, size_t size) = 0;
  uint mWidth = 0;
  uint mHeight = 0;
  uint mDepth = 0;
  mutable ShaderResourceView::sptr_t mSrv;
  eTextureFormat mFormat = TEXTURE_FORMAT_RGBA8;
};



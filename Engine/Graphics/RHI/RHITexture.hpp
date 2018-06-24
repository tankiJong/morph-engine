#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"

class RHITexture: public RHIResource, public inherit_shared_from_this<RHIResource, RHITexture> {
public:
  using sptr_t = std::shared_ptr<RHITexture>;
  using inherit_shared_from_this<RHIResource, RHITexture>::shared_from_this;

  static sptr_t create2D(uint width, uint height, const void* data, size_t size, BindingFlag flag = BindingFlag::ShaderResource);

  uint width() const { return mWidth; }
  uint height() const { return mHeight; }
protected:
  RHITexture(uint width, uint height, BindingFlag flags)
    : RHIResource(Type::Texture2D, flags), mWidth(width), mHeight(height) {}
  bool rhiInit(const void* data, size_t size);
  uint mWidth = 0;
  uint mHeight = 0;
};

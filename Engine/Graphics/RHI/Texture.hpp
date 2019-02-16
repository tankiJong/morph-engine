#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHITexture.hpp"
#include "Engine/Core/Resource.hpp"

class RenderTargetview;
class DepthStencilView;

class Texture2: public RHITexture, public inherit_shared_from_this<RHITexture, Texture2> {
public:
  using sptr_t = std::shared_ptr<Texture2>;
  using scptr_t = std::shared_ptr<const Texture2>;
  using inherit_shared_from_this<RHITexture, Texture2>::shared_from_this;

  virtual const RenderTargetView* rtv() const override;
  virtual const DepthStencilView* dsv() const override;

  virtual const UnorderedAccessView* uav() const override;
  static Texture2::sptr_t create(
    uint width, uint height, eTextureFormat format, 
    BindingFlag flag =  BindingFlag::ShaderResource, 
    const void* data = nullptr, size_t size = 0);
  static Texture2::sptr_t create(rhi_resource_handle_t res);


protected:
  mutable RenderTargetView::sptr_t mRtv;
  mutable DepthStencilView::sptr_t mDsv;
  template<typename TexType, typename ...Args>
  friend typename TexType::sptr_t createOrFail(const void* data, size_t size, Args ... args);
  friend class Resource<Texture2>;
  Texture2(uint width, uint height, eTextureFormat format,
           BindingFlag flag = BindingFlag::ShaderResource,
           const void* data = nullptr, size_t size = 0)
    :RHITexture(Type::Texture2D, width, height, 1, format, flag, data, size) {
  }
  Texture2(rhi_resource_handle_t res)
    :RHITexture(res) {}
  bool rhiInit(const void* data, size_t size) override;
};

class Texture3 : public RHITexture, public inherit_shared_from_this<RHITexture, Texture3> {
public:
  using sptr_t = std::shared_ptr<Texture3>;
  using inherit_shared_from_this<RHITexture, Texture3>::shared_from_this;

  Texture3::sptr_t create(uint width, uint height, uint depth,
                          BindingFlag flag = BindingFlag::ShaderResource,
                          const void* data = nullptr, size_t size = 0);
protected:
  template<typename TexType, typename ...Args>
  friend typename TexType::sptr_t createOrFail(const void* data, size_t size, Args ... args);
  Texture3(uint width, uint height, uint depth, eTextureFormat format, 
           BindingFlag flag = BindingFlag::ShaderResource,
           const void* data = nullptr, size_t size = 0)
    :RHITexture(Type::Texture2D, width, height, depth, format, flag, data, size) {};
  bool rhiInit(const void* data, size_t size) override;
};


template<>
ResDef<Texture2> Resource<Texture2>::load(const std::string& file);
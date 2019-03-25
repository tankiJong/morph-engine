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

  static sptr_t create(
    uint width, uint height, eTextureFormat format, 
    BindingFlag flag =  BindingFlag::ShaderResource, 
    const void* data = nullptr, size_t size = 0);

  static sptr_t create(
    uint width, uint height, eTextureFormat format, bool genMip,
    BindingFlag flag =  BindingFlag::ShaderResource, 
    const void* data = nullptr, size_t size = 0);
  static sptr_t create(rhi_resource_handle_t res);

protected:
  template<typename TexType, typename ...Args>
  friend typename TexType::sptr_t createOrFail(bool genMipmap, const void* data, size_t size, Args ... args);
  friend class Resource<Texture2>;
  Texture2(uint width, uint height, eTextureFormat format,
           BindingFlag flag = BindingFlag::ShaderResource,
           const void* data = nullptr, size_t size = 0)
    :RHITexture(Type::Texture2D, width, height, 1, 1, format, flag, data, size) {
  }
  Texture2(rhi_resource_handle_t res)
    :RHITexture(res) {}
  bool rhiInit(bool genMipmap, const void* data, size_t size) override;
};

class TextureCube: public RHITexture, public inherit_shared_from_this<RHITexture, TextureCube> {
  friend class VoxelRenderer;
public:
  using sptr_t = std::shared_ptr<TextureCube>;
  using scptr_t = std::shared_ptr<const TextureCube>;
  using inherit_shared_from_this<RHITexture, TextureCube>::shared_from_this;

  static sptr_t create(
    uint width, uint height, eTextureFormat format, bool genMipMap = true,
    BindingFlag flag = BindingFlag::ShaderResource,
    const void* data = nullptr, size_t size = 0);

protected:
  template<typename TexType, typename ...Args>
  friend typename TexType::sptr_t createOrFail(bool genMipmap, const void* data, size_t size, Args ... args);
  friend class Resource<TextureCube>;
  TextureCube(uint width, uint height, eTextureFormat format,
              BindingFlag flag = BindingFlag::ShaderResource,
              const void* data = nullptr, size_t size = 0)
    :RHITexture(Type::TextureCube, width, height, 1, 1, format, flag, data, size) {}

  bool rhiInit(bool genMipmap, const void* data, size_t size) override;
};

class Texture3 : public RHITexture, public inherit_shared_from_this<RHITexture, Texture3> {
public:
  using sptr_t = std::shared_ptr<Texture3>;
  using inherit_shared_from_this<RHITexture, Texture3>::shared_from_this;

  static Texture3::sptr_t create(uint width, uint height, uint depth, eTextureFormat format,
                          BindingFlag flag = BindingFlag::ShaderResource,
                          const void* data = nullptr, size_t size = 0);
protected:
  template<typename TexType, typename ...Args>
  friend typename TexType::sptr_t createOrFail(bool genMipmap, const void* data, size_t size, Args ... args);
  Texture3(uint width, uint height, uint depth, eTextureFormat format, 
           BindingFlag flag = BindingFlag::ShaderResource,
           const void* data = nullptr, size_t size = 0)
    :RHITexture(Type::Texture2D, width, height, depth, 1, format, flag, data, size) {};
  bool rhiInit(bool genMipmap, const void* data, size_t size) override;
};


template<>
ResDef<Texture2> Resource<Texture2>::load(const std::string& file);

template<>
ResDef<TextureCube> Resource<TextureCube>::load(const std::string& file);
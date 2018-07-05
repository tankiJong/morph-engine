#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHITexture.hpp"


class Texture2: public RHITexture, public inherit_shared_from_this<RHITexture, Texture2> {
public:
  using sptr_t = std::shared_ptr<Texture2>;
  using inherit_shared_from_this<RHITexture, Texture2>::shared_from_this;

  static Texture2::sptr_t create(uint width, uint height,
                          BindingFlag flag = BindingFlag::ShaderResource,
                          const void* data = nullptr, size_t size = 0);

protected:
  template<typename TexType, typename ...Args>
  friend typename TexType::sptr_t createOrFail(const void* data, size_t size, Args ... args);
  Texture2(uint width, uint height,
           BindingFlag flag = BindingFlag::ShaderResource,
           const void* data = nullptr, size_t size = 0)
    :RHITexture(Type::Texture2D, width, height, 1, flag, data, size) {
  }
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
  Texture3(uint width, uint height, uint depth,
           BindingFlag flag = BindingFlag::ShaderResource,
           const void* data = nullptr, size_t size = 0)
    :RHITexture(Type::Texture2D, width, height, depth, flag, data, size) {};
  bool rhiInit(const void* data, size_t size) override;
};


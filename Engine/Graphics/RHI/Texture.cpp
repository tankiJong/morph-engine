#include "Texture.hpp"

template<typename TexType, typename ...Args>
typename TexType::sptr_t createOrFail(const void* data, size_t size, Args ... args) {
  typename TexType::sptr_t tex = TexType::sptr_t(new TexType(args..., data, size));
  return tex->rhiInit(data, size) ? tex : nullptr;
}

Texture2::sptr_t Texture2::create(uint width, uint height, BindingFlag flag, const void* data, size_t size) {
  return createOrFail<Texture2>(data, size, width, height, flag);
}
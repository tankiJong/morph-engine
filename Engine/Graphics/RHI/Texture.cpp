#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Texture.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"

template<typename TexType, typename ...Args>
typename TexType::sptr_t createOrFail(bool genMipmap, const void* data, size_t size, Args ... args) {
  typename TexType::sptr_t tex = TexType::sptr_t(new TexType(args..., data, size));
  return tex->rhiInit(genMipmap, data, size) ? tex : nullptr;
}


Texture2::sptr_t Texture2::create(
  uint width, uint height, eTextureFormat format, 
  BindingFlag flag, const void* data, size_t size) {
  return createOrFail<Texture2>(false, data, size, width, height, format, flag);
}

Texture2::sptr_t Texture2::create(
  uint width, uint height, eTextureFormat format, bool genMip,
  BindingFlag flag, const void* data, size_t size) {
  return createOrFail<Texture2>(genMip, data, size, width, height, format, flag);
}

Texture2::sptr_t Texture2::create(rhi_resource_handle_t res) {
  return Texture2::sptr_t(new Texture2(res));
}



template<>
ResDef<Texture2> Resource<Texture2>::load(const std::string& file) {
  auto name = make_wstring(file);
  if (file == "$default") {
    Texture2* tex = new Texture2(1, 1, TEXTURE_FORMAT_RGBA8, RHIResource::BindingFlag::ShaderResource);
    tex->rhiInit(false, &Rgba::white, sizeof(vec4));
    setName(*tex, name.c_str());
    return { file, tex };
  }

  FileSystem& vfs = FileSystem::Get();
  auto realPath = vfs.locate(file);

  if (!realPath) return { file, nullptr };

  Image img(realPath->string());

  Texture2* tex = new Texture2(img.dimension().x, img.dimension().y, 
                               img.format(), RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::RenderTarget);

  // hack to make shared from this works
  S<Texture2> sTex(tex, [](Texture2*) {});
  tex->rhiInit(true, img.data(), img.size());
  tex->invalidateViews();
  setName(*tex, name.c_str());
  return { file, tex };
};
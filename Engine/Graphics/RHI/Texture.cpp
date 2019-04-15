#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Texture.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Math/Primitives/aabb2.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Renderer/Texture.hpp"

template<typename TexType, typename ...Args>
typename TexType::sptr_t createOrFail(bool impliciteHeap, bool genMipmap, const void* data, size_t size, Args ... args) {
  auto tex = TexType::sptr_t(new TexType(args..., data, size));
  return tex->rhiInit(genMipmap, data, size, impliciteHeap) ? tex : nullptr;
}


Texture2::sptr_t Texture2::create(
  uint width, uint height, eTextureFormat format, 
  BindingFlag flag, const void* data, size_t size, bool implicitHeap) {
  return createOrFail<Texture2>(implicitHeap, false, data, size, width, height, format, flag);
}

Texture2::sptr_t Texture2::create(
  uint width, uint height, eTextureFormat format, bool genMip,
  BindingFlag flag, const void* data, size_t size, bool implicitHeap) {
  return createOrFail<Texture2>(implicitHeap, genMip, data, size, width, height, format, flag);
}

Texture2::sptr_t Texture2::create(rhi_resource_handle_t res) {
  return Texture2::sptr_t(new Texture2(res));
}

Texture3::sptr_t Texture3::create(uint width, uint height, uint depth, eTextureFormat format,
                                  BindingFlag flag, const void* data, size_t size) {
  return createOrFail<Texture3>(true, false, data, size, width, height, depth, format, flag);
}

Texture3::sptr_t Texture3::create(uint width, uint height, uint depth,
                                  eTextureFormat format, bool genMipmaps, 
	                                BindingFlag flag, const void* data, size_t size) {
  return createOrFail<Texture3>(true, genMipmaps, data, size, width, height, depth, format, flag);
}

TextureCube::sptr_t TextureCube::create(
  uint width, uint height, eTextureFormat format,
  bool genMipMap, BindingFlag flag,
  const void* data, size_t size) {
  return createOrFail<TextureCube>(true, genMipMap, data, size,  width, height, format, flag);
  
}


template<>
ResDef<Texture2> Resource<Texture2>::load(const std::string& file) {
  auto name = make_wstring("Texture2: " + file);
  if (file == "$default") {
    Texture2* tex = new Texture2(1, 1, TEXTURE_FORMAT_RGBA8, RHIResource::BindingFlag::ShaderResource);
    tex->rhiInit(false, &Rgba::white, sizeof(vec4), true);
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
  tex->rhiInit(true, img.data(), img.size(), true);
  tex->invalidateViews();
  setName(*tex, name.c_str());
  return { file, tex };
};

template<>
ResDef<TextureCube> Resource<TextureCube>::load(const std::string& file) {
  FileSystem& vfs = FileSystem::Get();
  auto realPath = vfs.locate(file);
  if(!realPath) return {file, nullptr};

  // cube map index sequence:  [+X, ¨CX, +Y, ¨CY, +Z, ¨CZ] 
  //          top
  //  left    front    right      back
  //          bottom

  static const aabb2 uvregions[6] = {
    { {.5f, 1.f/3.f}, {.75f, 2.f/3.f} }, // right
    { {.0f, 1.f/3.f}, {.25f, 2.f/3.f} }, // left
    { {.25f, .0f}, {.5f, 1.f/3.f} }, // top
    { {.25f, 2.f/3.f}, {.5f, 1.f} }, // btm
    { {.25f, 1.f/3.f}, {.5f, 2.f/3.f} }, // front
    { {.75f, 1.f/3.f}, {1.f, 2.f/3.f} }, // back
  };

  auto texture = Resource<Texture2>::get(file);

  uvec2 texSize = texture->size(0);
  TextureCube* cube = new TextureCube(
    texSize.x / 4u, texSize.y / 3u,
    texture->format(),
    RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::RenderTarget);

    // hack to make shared from this works
  S<TextureCube> sTex(cube, [](TextureCube*) {});
  cube->rhiInit(false, nullptr, 0, true);
  ShaderResourceView* srv = texture->srv(0);
  for(uint i = 0; i < 6; i++) {
    const RenderTargetView* rtv = cube->rtv(0, i);  
    RHIDevice::get()->defaultRenderContext()->blit(*srv, *rtv, uvregions[i], { vec2::zero, vec2::one});
  }
  cube->invalidateViews();

  auto name = make_wstring("TexutreCube: " + file);
  setName(*cube, name.c_str());

  return { file, cube };
}

#include "TextureArray.hpp"
#include "Engine/Renderer/glFunctions.hpp"

TextureArray::TextureArray(uvec2 dimen, uint layerCount, eTextureFormat format)
  : mDimension(dimen, layerCount)
  , mFormat(format){
  setup();
}

void TextureArray::setLayer(Texture* tex, uint layer) {
  glBindTexture(GL_TEXTURE_2D, tex->getHandle());
  glCopyTexSubImage3D(mHandle, 0, 0, 0, layer, 0, 0, tex->dimension().x, tex->dimension().y);
}

void TextureArray::setup() {
  if (mHandle != 0)return;

  glGenTextures(1, &mHandle);

  glBindTexture(GL_PROXY_TEXTURE_2D_ARRAY, mHandle);

  GLenum internal_format = GL_RGBA8;
  GLenum channels = GL_RGBA;
  GLenum pixel_layout = GL_UNSIGNED_BYTE;
  GLTexFormat(mFormat, internal_format, channels, pixel_layout);

  glTexStorage3D(GL_PROXY_TEXTURE_2D_ARRAY,
                 1, // mip-level
                 internal_format,
                 mDimension.x,
                 mDimension.y,
                 mDimension.z);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

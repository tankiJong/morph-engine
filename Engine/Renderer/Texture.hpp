//-----------------------------------------------------------------------------------------------
// Texture.hpp
//
#pragma once
#include <string>
#include "Engine/Math/Primitives/ivec2.hpp"
#include "Engine/Core/common.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/type.h"
#include "Engine/Core/Resource.hpp"

//---------------------------------------------------------------------------

class RenderTarget;
// for now, texture manangement is none meta file based.


class Texture {
	friend class Renderer;
	friend class FrameBuffer;

public:
  Texture(eTextureFormat format = TEXTURE_FORMAT_RGBA8);
  Texture(uint width, uint height, eTextureFormat format);
  Texture(const std::string& imageFilePath);
  Texture(const Image& image);

  inline ivec2 dimension() const { return mDimensions; };
  inline eTextureFormat format() const { return mFormat; }
  inline uint getHandle() const { return mTextureID; }
 
  void resize(ivec2 size);
  RenderTarget& asRenderTarget() { return (RenderTarget&)*this; }
  const RenderTarget& asRenderTarget() const { return (const RenderTarget&)*this; }
  virtual ~Texture();

protected:
  void fromImage(const Image& image);

  void init(uint width, uint height, eTextureFormat format);
  void PopulateFromData();

  Texture* clone() const;

protected:
  unsigned int   mTextureID;
  Image*         mData = nullptr;
  ivec2          mDimensions;
  eTextureFormat mFormat;
};

template<>
ResDef<Texture> Resource<Texture>::load(const std::string& file);

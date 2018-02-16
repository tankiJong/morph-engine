//-----------------------------------------------------------------------------------------------
// Texture.hpp
//
#pragma once
#include "Engine/Math/Ivec2.hpp"
#include <string>
#include <map>
#include "Engine/Core/common.hpp"
#include "Engine/Core/Image.hpp"
//---------------------------------------------------------------------------

enum eTextureFormat {
  TEXTURE_FORMAT_RGBA8, // default color format
  TEXTURE_FORMAT_D24S8,
};
class Texture
{
	friend class Renderer; // Textures are managed by a Renderer instance
  friend class FrameBuffer;
private:
  Texture();
	Texture( const std::string& imageFilePath );
  Texture(const Image& image);
  void fromImage(const Image& image);
	void PopulateFromData();
  bool setupRenderTarget(uint width, uint height, eTextureFormat format);
  inline uint getHandle() const { return mTextureID; }
private:
	unsigned int								mTextureID;
  Image                       mData;
	ivec2									mDimensions;
  eTextureFormat mFormat;
};



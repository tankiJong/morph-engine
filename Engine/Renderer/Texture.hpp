//-----------------------------------------------------------------------------------------------
// Texture.hpp
//
#pragma once
#include <map>
#include <string>
#include "Engine/Math/Primitives/ivec2.hpp"
#include "Engine/Core/common.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/type.h"
//---------------------------------------------------------------------------


class Texture
{
	friend class Renderer; // Textures are managed by a Renderer instance
  friend class FrameBuffer;
public:
  inline ivec2 dimension() const { return mDimensions; };
private:
  Texture();
	Texture( const std::string& imageFilePath );
  Texture(const Image& image);
  void fromImage(const Image& image);
	void PopulateFromData();
  bool setupRenderTarget(uint width, uint height, eTextureFormat format);
  inline uint getHandle() const { return mTextureID; }
  Texture* clone() const;
private:
	unsigned int	 mTextureID;
  Image          mData;
	ivec2					 mDimensions;
  eTextureFormat mFormat;
};



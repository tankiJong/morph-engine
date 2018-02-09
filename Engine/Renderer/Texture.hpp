//-----------------------------------------------------------------------------------------------
// Texture.hpp
//
#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <string>
#include <map>
#include "Engine/Core/common.hpp"
#include "Engine/Core/Image.hpp"
//---------------------------------------------------------------------------

enum eTextureFormat {
  
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
  void setupRenderTarget(uint width, uint height, eTextureFormat format);
  inline uint getHandle() const { return mTextureID; }
private:
	unsigned int								mTextureID;
  Image                       mData;
	IntVec2									mDimensions;
};



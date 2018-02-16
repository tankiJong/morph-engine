//-----------------------------------------------------------------------------------------------
// Texture.cpp
//
#include "Engine/Renderer/Texture.hpp"
#include "glFunctions.hpp"
#include "Engine/Core/Rgba.hpp"

Texture::Texture()
: mTextureID(0)
, mData(&Rgba::white, 1, 1)
, mDimensions(1,1){
}
//-----------------------------------------------------------------------------------------------
// Called only by the Renderer.  Use renderer->CreateOrGetTexture() to instantiate textures.
//
Texture::Texture( const std::string& imageFilePath )
	: mTextureID( 0 )
  , mData(imageFilePath)
	, mDimensions( mData.dimension() ) {

  PopulateFromData();
}

Texture::Texture(const Image& image) 
  : mTextureID(0)
  , mData(image)
  , mDimensions(mData.dimension()) {
  PopulateFromData();
}

void Texture::fromImage(const Image& image) {
  mData = image;
  PopulateFromData();
}

//-----------------------------------------------------------------------------------------------
// Creates a texture identity on the video card, and populates it with the given image texel data
//
void Texture:: PopulateFromData()
{
  unsigned char* imageData = (unsigned char*)mData.data();
  int numComponents = 4;
	// Enable texturing
//	glEnable( GL_TEXTURE_2D );
//  GL_CHECK_ERROR();
	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  GL_CHECK_ERROR();

  if( mTextureID == NULL) {
	  // Ask OpenGL for an unused texName (ID number) to use for this texture
	  glGenTextures( 1, (GLuint*) &mTextureID );
  }

  GL_CHECK_ERROR();
	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, mTextureID );

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if( numComponents == 3 )
		bufferFormat = GL_RGB;

	GLenum internalFormat = bufferFormat; // the format we want the texture to be on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		mDimensions.x,			// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,11], and B is the border thickness [0,1]
		mDimensions.y,			// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,11], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1, recommend 0)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color channel/component)
		imageData );		// Address of the actual pixel data bytes/buffer in system memory
}

bool Texture::setupRenderTarget(uint width, uint height, eTextureFormat format) {
  // generate the link to this texture
  glGenTextures(1, &mTextureID);
  if (mTextureID == NULL) {
    return false;
  }

  // TODO - add a TextureFormatToGLFormats( GLenum*, GLenum*, GLenum*, eTextureFormat )
  //        when more texture formats are required; 
  GLenum internal_format = GL_RGBA8;
  GLenum channels = GL_RGBA;
  GLenum pixel_layout = GL_UNSIGNED_BYTE;
  if (format == TEXTURE_FORMAT_D24S8) {
    internal_format = GL_DEPTH_STENCIL;
    channels = GL_DEPTH_STENCIL;
    pixel_layout = GL_UNSIGNED_INT_24_8;
  }

  // Copy the texture - first, get use to be using texture unit 0 for this; 
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mTextureID);    // bind our texture to our current texture unit (0)

                                             // Copy data into it;
  glTexImage2D(GL_TEXTURE_2D, 0,
               internal_format, // what's the format OpenGL should use
               width,
               height,
               0,             // border, use 0
               channels,      // how many channels are there?
               pixel_layout,  // how is the data laid out
               nullptr);     // don't need to pass it initialization data 

                             // make sure it suceeded
  GL_CHECK_ERROR();
  // cleanup after myself; 
  glBindTexture(GL_TEXTURE_2D, NULL); // unset it; 

                                      // Save this all off
  mDimensions.x = width;
  mDimensions.y = height;

  mFormat = format; // I save the format with the texture
                  // for sanity checking.

                  // great, success
  return true;
}


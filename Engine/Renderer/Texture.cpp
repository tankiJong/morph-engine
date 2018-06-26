//-----------------------------------------------------------------------------------------------
// Texture.cpp
//
#include "Engine/Renderer/Texture.hpp"
#include "glFunctions.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/File/FileSystem.hpp"

uint calculateMipCount(uint size) {
  uint i = 0;
	while(size > 0) {
    size >>= 1;
    i++;
	}
  return i;
}

Texture::Texture(eTextureFormat format)
	: mTextureID(0)
	, mData(new Image(&Rgba::white, 1, 1))
	, mDimensions(1,1)
	, mFormat(format) {
	PopulateFromData();
}

Texture::Texture(uint width, uint height, eTextureFormat format)
	: mTextureID(0) 
	, mData(nullptr) 
	, mDimensions(width, height) 
	, mFormat(format) {}

Texture::Texture( const std::string& imageFilePath )
  : mTextureID( 0 )
  , mData(new Image(imageFilePath))
  , mDimensions( mData->dimension() ) {

  PopulateFromData();
}

Texture::Texture(const Image& image) 
  : mTextureID(0)
  , mData(new Image(image))
  , mDimensions(mData->dimension()) {
  PopulateFromData();
}

void Texture::resize(ivec2 size) {
	if (mTextureID == NULL) return;
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	GLenum internal_format = GL_RGBA8;
	GLenum channels = GL_RGBA;
	GLenum pixel_layout = GL_UNSIGNED_BYTE;
	GLTexFormat(mFormat, internal_format, channels, pixel_layout);

	glTexImage2D(			
				GL_TEXTURE_2D,		
				0,
				internal_format,
				mDimensions.x,
				mDimensions.y,
				0,	
				channels,	
				pixel_layout,
				0);

	GL_CHECK_ERROR();

}

Texture::~Texture() {
	glDeleteTextures(1, &mTextureID);
}

void Texture::fromImage(const Image& image) {
  mData = new Image(image);
  PopulateFromData();
}

void Texture::init(uint width, uint height, eTextureFormat format) {
	
}

//-----------------------------------------------------------------------------------------------
// Creates a texture identity on the video card, and populates it with the given image texel data
//
void Texture:: PopulateFromData()
{
  unsigned char* imageData = mData ? (unsigned char*)mData->data() : nullptr;
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
	if( numComponents == 3 ) {
		bufferFormat = GL_RGB;
	}

	ENSURES(numComponents == 3 || numComponents == 4);
	GLenum internalFormat = bufferFormat; // the format we want the texture to be on the card; allows us to translate into a different texture format as we upload to OpenGL

  uint mipCount = calculateMipCount(max(mDimensions.x, mDimensions.y));
  glTexStorage2D(GL_TEXTURE_2D,
                 mipCount,       // number of levels (mip-layers)           CHANGED FROM ONE!!
                 numComponents == 3 ? GL_RGB8 : GL_RGBA8, // how is the memory stored on the GPU
                 mDimensions.x, mDimensions.y); // dimenions

  GL_CHECK_ERROR();

  glTexSubImage2D(
    GL_TEXTURE_2D,
    0,
    0, 0,
    mDimensions.x, mDimensions.y,
    bufferFormat,
    GL_UNSIGNED_BYTE,
    imageData);
  //
  // glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		// GL_TEXTURE_2D,		// Creating this as a 2d texture
		// 0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
  //              bufferFormat,
  //              		// Type of texel format we want OpenGL to use for this texture internally on the video card
		// mDimensions.x,			// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,11], and B is the border thickness [0,1]
		// mDimensions.y,			// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,11], and B is the border thickness [0,1]
		// 0,					// Border size, in texels (must be 0 or 1, recommend 0)
  //              bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		// GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color channel/component)
		// imageData );		// Address of the actual pixel data bytes/buffer in system memory
  GL_CHECK_ERROR();

  glGenerateMipmap(GL_TEXTURE_2D);
}

Texture* Texture::clone() const {
  Texture* tex = new Texture();
  tex->mData = mData;
  tex->mDimensions = mDimensions;
  tex->mFormat = mFormat;
  return tex;
}

template<>
ResDef<Texture> Resource<Texture>::load(const std::string& file) {
	if(file == "$default") {
		Texture* tex = new Texture();
		return { file, tex };
	}
	FileSystem& vfs = FileSystem::Get();
	auto realPath = vfs.locate(file);

	if (!realPath) return { file, nullptr };

	Image img(realPath->string());

	Texture* tex = new Texture(img);

	return { file, tex };
};
#include "RenderTarget.hpp"
#include "Engine/Renderer/glFunctions.hpp"

RenderTarget::RenderTarget(uint width, uint height, eTextureFormat format): Texture(width, height, format) {
  setup(width, height, format);
}

bool RenderTarget::setup(uint width, uint height, eTextureFormat format) {
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
  GLTexFormat(format, internal_format, channels, pixel_layout);
//  if (format == TEXTURE_FORMAT_D24S8) {
//    internal_format = GL_DEPTH_STENCIL;
//    channels = GL_DEPTH_STENCIL;
//    pixel_layout = GL_UNSIGNED_INT_24_8;
//  }

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
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
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


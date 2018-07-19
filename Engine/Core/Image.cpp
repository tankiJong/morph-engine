#pragma warning( push, 0 )
#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"
#pragma warning( pop )  
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Primitives/uvec2.hpp"
#include "Image.hpp"
#include "Engine/Math/Primitives/Ivec2.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Async/Thread.hpp"

Image::Image(const std::string& imageFilePath) {
  int numComponents = 0;
  int numComponentsRequested = 0;

  stbi_set_flip_vertically_on_load(true);
  ivec2 dimension(mDimensions);
  unsigned char* imageData = stbi_load(imageFilePath.c_str(), &dimension.x, &dimension.y, &numComponents, numComponentsRequested);
  mDimensions.x = dimension.x;
  mDimensions.y = dimension.y;
  populateFromData(imageData, mDimensions, numComponents);
  stbi_image_free(imageData);
}

Image::Image(const Rgba* data, uint width, uint height)
  : mDimensions(width, height) 
  , mTexels(data, data + width * height){
}

Image::Image(uint width, uint height) 
  : mDimensions(width, height) {
  mTexels.resize(width * height);
}

Rgba Image::getTexel(int x, int y) const {
  return mTexels[x + y * mDimensions.x];
}

Rgba Image::getTexel(uvec2 v) const {
  return getTexel(v.x, v.y);
}

void Image::setTexel(int x, int y, const Rgba& color) {
  mTexels[x + y * mDimensions.x] = color;
}

void Image::populateFromData(unsigned char* imageData, const uvec2& dimensions, int numComponents) {
  if(numComponents == 4) {
    Rgba* colorData = (Rgba*)imageData;

    int numTexel = dimensions.x * dimensions.y;

    for(int i = 0; i<numTexel;i++) {
      mTexels.push_back(Rgba(*colorData));
      colorData++;
    }

    return;
  }

  if(numComponents == 3) {
    struct rgb {
      unsigned char r;
      unsigned char g;
      unsigned char b;
    };

    rgb* colorData = (rgb*)imageData;

    int numTexel = dimensions.x * dimensions.y;

    // TODO: haven't do the validation
    for (int i = 0; i<numTexel; i++) {
      mTexels.emplace_back(colorData->r, colorData->g, colorData->b, unsigned char(255));
      colorData++;
    }

    return;
  }

  if(numComponents == 1) {
    unsigned char* colorData = (unsigned char*)imageData;
    int numTexel = dimensions.x * dimensions.y;
    // TODO: haven't do the validation
    for (int i = 0; i<numTexel; i++) {
      mTexels.emplace_back(*colorData, (unsigned char)0, (unsigned char)0, unsigned char(255));
      colorData++;
    }

    return;
  }
}

Rgba& Image::operator()(uint x, uint y) {
  EXPECTS(x * y <= mDimensions.x * mDimensions.y);
  return mTexels[x + y * mDimensions.x];
}

bool Image::save(const char* path) {
  stbi_flip_vertically_on_write(1);
  return stbi_write_bmp(path, (int)mDimensions.x, (int)mDimensions.y, 4, mTexels.data()) == 0;
}

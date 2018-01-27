#pragma warning( push )
#pragma warning( disable : 4456 ) 
#pragma warning( disable : 4457 ) 
#include "ThirdParty/stb/stb_image.h"
#pragma warning( pop )  
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Image.hpp"

Image::Image(const std::string& imageFilePath) {
  int numComponents = 0;
  int numComponentsRequested = 0;

  unsigned char* imageData = stbi_load(imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, numComponentsRequested);
  populateFromData(imageData, m_dimensions, numComponents);
  stbi_image_free(imageData);
}

Rgba Image::getTexel(int x, int y) const {
  return m_texels[x + y * m_dimensions.x];
}

Rgba Image::getTexel(IntVector2 v) const {
  return getTexel(v.x, v.y);
}

void Image::setTexel(int x, int y, const Rgba& color) {
  m_texels[x + y * m_dimensions.x] = color;
}

void Image::populateFromData(unsigned char* imageData, const IntVector2& dimensions, int numComponents) {
  if(numComponents == 4) {
    Rgba* colorData = (Rgba*)imageData;

    int numTexel = dimensions.x * dimensions.y;

    for(int i = 0; i<numTexel;i++) {
      m_texels.push_back(Rgba(*colorData));
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
      m_texels.push_back(Rgba(colorData->r, colorData->g, colorData->b, 255));
      colorData++;
    }

    return;
  }
}

#pragma once
#include <vector>
#include "Engine/Core/common.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Ivec2.hpp"

class Image {
public:
  Image() {}
  Image(const std::string& imageFilePath);
  Image(const Rgba* data, uint width, uint height);
  Rgba getTexel(int x, int y) const; // (0,0) is top-left
  Rgba getTexel(ivec2 v) const;
  void setTexel(int x, int y, const Rgba& color);
  void populateFromData(unsigned char* imageData, const ivec2& dimensions, int numComponents);

  inline ivec2 dimension() const { return mDimensions; }
  inline Rgba* data() { return mTexels.data(); }
  inline const Rgba* data() const { return mTexels.data(); }

private:
  ivec2 mDimensions;
  std::vector<Rgba> mTexels;
};

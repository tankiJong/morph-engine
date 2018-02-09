#pragma once
#include <vector>
#include "Engine/Core/common.hpp"
#include "Engine/Core/Rgba.hpp"
class IntVec2;

class Image {
public:
  Image() {}
  Image(const std::string& imageFilePath);
  Image(const Rgba* data, uint width, uint height);
  Rgba getTexel(int x, int y) const; // (0,0) is top-left
  Rgba getTexel(IntVec2 v) const;
  void setTexel(int x, int y, const Rgba& color);
  void populateFromData(unsigned char* imageData, const IntVec2& dimensions, int numComponents);

  inline IntVec2 dimension() const { return mDimensions; }
  inline Rgba* data() { return mTexels.data(); }
  inline const Rgba* data() const { return mTexels.data(); }

private:
  IntVec2 mDimensions;
  std::vector<Rgba> mTexels;
};

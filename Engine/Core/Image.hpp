#pragma once
#include <vector>
#include "Engine/Core/common.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Primitives/uvec2.hpp"

class Image {
public:
  Image() {}
  Image(const std::string& imageFilePath);
  Image(const Rgba* data, uint width, uint height);
  Rgba getTexel(int x, int y) const; // (0,0) is top-left
  Rgba getTexel(uvec2 v) const;
  void setTexel(int x, int y, const Rgba& color);
  void populateFromData(unsigned char* imageData, const uvec2& dimensions, int numComponents);

  inline uvec2 dimension() const { return mDimensions; }
  inline Rgba* data() { return mTexels.data(); }
  inline const Rgba* data() const { return mTexels.data(); }
  inline uint size() const { return mDimensions.x * mDimensions.y; };

  Rgba& operator()(uint x, uint y);
private:
  uvec2 mDimensions;
  std::vector<Rgba> mTexels;
};

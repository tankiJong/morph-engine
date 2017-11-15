#pragma once
#include <vector>

class Rgba;
class IntVector2;

class Image {
public:
  Image() {}
  explicit Image(const std::string& imageFilePath);
  Rgba	getTexel(int x, int y) const; 			// (0,0) is top-left
  void	setTexel(int x, int y, const Rgba& color);
  void populateFromData(unsigned char* imageData, const IntVector2& dimensions, int numComponents);

  private:
  IntVector2		m_dimensions;
  std::vector<Rgba>	m_texels;
};
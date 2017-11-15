#pragma once
#include <string>

class Rgba {
public:
	Rgba() {};
	~Rgba() {};
	explicit Rgba(unsigned char redByte, unsigned char greenByte,
						unsigned char blueByte, unsigned char alphaByte = 255);
	void setByBytes(unsigned char redByte, unsigned char greenByte,
						  unsigned char blueByte, unsigned char alphaByte = 255);

	void setByFloats(float normalizedRed, float normalizedGreen,
						   float normalizedBlue, float normalizedAlpha = 1.0f);

	void getAsFloats(float& out_normalizedRed, float& out_normalizedGreen,
						   float& out_normalizedBlue, float& out_normalizedAlpha) const;

	void scaleColor(float rgbScale);
	void scaleOpacity(float alphaScale);
  void fromString(const char* data);
  void fromHexString(const char* data);
  void fromRgbString(const char* data);
  std::string toString(bool withAlpha = true);
  const static Rgba white;
  const static Rgba red;
public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;
};



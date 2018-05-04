#pragma once
#include <string>
#include "Engine/Math/Primitives/vec4.hpp"

class Rgba {
public:
	Rgba() = default;;
	~Rgba() = default;;
	explicit Rgba(unsigned char redByte, unsigned char greenByte,
						unsigned char blueByte, unsigned char alphaByte = 255);
	void setByBytes(unsigned char redByte, unsigned char greenByte,
						  unsigned char blueByte, unsigned char alphaByte = 255);

	void setByFloats(float normalizedRed, float normalizedGreen,
						   float normalizedBlue, float normalizedAlpha = 1.0f);

	void getAsFloats(float& out_normalizedRed, float& out_normalizedGreen,
						   float& out_normalizedBlue, float& out_normalizedAlpha) const;

  vec4 normalized() const;
	void scaleColor(float rgbScale);
	void scaleOpacity(float alphaScale);
  void fromString(const char* data);
  void fromHexString(const char* data);
  void fromRgbString(const char* data);
  Rgba operator*(float rhs) const;
  friend Rgba operator*(float, const Rgba&);
  std::string toString(bool withAlpha = true);
  const static Rgba white;
  const static Rgba red;
  const static Rgba cyan;
  const static Rgba black;
  const static Rgba yellow;
  const static Rgba transparent;
  const static Rgba blue;
  const static Rgba gray;
	const static Rgba green;
	const static Rgba magenta;
public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;
};

Rgba operator*(float lhs, const Rgba& rhs);
Rgba Hsl(float h, float s, float l);
Rgba Hue(unsigned char h);


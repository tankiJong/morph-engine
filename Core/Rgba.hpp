#pragma once

class Rgba {
public:
	Rgba() {};
	~Rgba() {};
	explicit Rgba::Rgba(unsigned char redByte, unsigned char greenByte,
						unsigned char blueByte, unsigned char alphaByte = 255);
	void Rgba::setByBytes(unsigned char redByte, unsigned char greenByte,
						  unsigned char blueByte, unsigned char alphaByte = 255);

	void Rgba::setByFloats(float normalizedRed, float normalizedGreen,
						   float normalizedBlue, float normalizedAlpha = 1.0f);

	void Rgba::getAsFloats(float& out_normalizedRed, float& out_normalizedGreen,
						   float& out_normalizedBlue, float& out_normalizedAlpha) const;

	void Rgba::scaleColor(float rgbScale);
	void Rgba::scaleOpacity(float alphaScale);


public:
	unsigned char r = 255;
	unsigned char b = 255;
	unsigned char a = 255;
	unsigned char g = 255;
};



#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

const Rgba Rgba::white = Rgba(255, 255, 255);
const Rgba Rgba::red = Rgba(255, 0  , 0  );

Rgba::Rgba(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte)
	: r(redByte)
	, g(greenByte)
	, b(blueByte)
	, a(alphaByte) {}

void Rgba::setByBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte) {
	r = redByte;
	g = greenByte;
	b = blueByte;
	a = alphaByte;
}

void Rgba::setByFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha) {
	r = unsigned char(normalizedRed * 255);
	g = unsigned char(normalizedGreen * 255);
	b = unsigned char(normalizedBlue * 255);
	a = unsigned char(normalizedAlpha * 255);
}

void Rgba::getAsFloats(float & out_normalizedRed, float & out_normalizedGreen, float & out_normalizedBlue, float & out_normalizedAlpha) const {
	out_normalizedRed = 1.f / 255.f * (float)r;
	out_normalizedGreen = 1.f / 255.f * (float)g;
	out_normalizedBlue = 1.f / 255.f * (float)b;
	out_normalizedAlpha = 1.f / 255.f * (float)a;
}

void Rgba::scaleColor(float rgbScale) {
	float _r = r * rgbScale;
	float _g = g * rgbScale;
	float _b = b * rgbScale;

	r = unsigned char(clampf(_r, 0, 255));
	g = unsigned char(clampf(_g, 0, 255));
	b = unsigned char(clampf(_b, 0, 255));
}

void Rgba::scaleOpacity(float alphaScale) {
	float _a = a * alphaScale;
	a = unsigned char(clampf(_a, 0, 255));
}

void Rgba::fromString(const char* data) {
  auto rgba = split(data, " ,");

  GUARANTEE_RECOVERABLE(rgba.size() == 3 || rgba.size() == 4, "try to cast illegal string to Rgba");

  if(rgba.size() == 3) {
    setByBytes(
      parse<unsigned char>(rgba[0]), 
      parse<unsigned char>(rgba[1]), 
      parse<unsigned char>(rgba[2]));
  } else if(rgba.size() == 4) {
    setByBytes(
      parse<unsigned char>(rgba[0]), 
      parse<unsigned char>(rgba[1]), 
      parse<unsigned char>(rgba[2]), 
      parse<unsigned char>(rgba[3]));
  }

}

std::string Rgba::toString(bool withAlpha) {
  if(withAlpha) {
    return Stringf("%u,%u,%u,%u", r, g, b, a);
  } else {
    return Stringf("%u,%u,%u", r, g, b);
  }
}


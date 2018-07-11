#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

const Rgba Rgba::white = Rgba(255, 255, 255);
const Rgba Rgba::black = Rgba(0  , 0  , 0  );
const Rgba Rgba::red   = Rgba(255, 0  , 0  );
const Rgba Rgba::green = Rgba(0  , 255, 0  );
const Rgba Rgba::blue  = Rgba(0  , 0  , 255);
const Rgba Rgba::yellow= Rgba(255, 255, 0  );
const Rgba Rgba::cyan  = Rgba(0  , 255, 255);
const Rgba Rgba::gray  = Rgba(128, 128, 128);
const Rgba Rgba::magenta = Rgba(255, 0, 255);

const Rgba Rgba::transparent = Rgba(255, 255, 255, 0);

Rgba::Rgba(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte)
	: r(redByte)
	, g(greenByte)
	, b(blueByte)
	, a(alphaByte) {}

Rgba::Rgba(const vec3& rgb, float a) {
  setByFloats(rgb.x, rgb.y, rgb.z, a);
}

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

vec4 Rgba::normalized() const {
  return {
    1.f / 255.f * (float)r, 
    1.f / 255.f * (float)g, 
    1.f / 255.f * (float)b, 
    1.f / 255.f * (float)a, 
  };
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

void Rgba::fromRgbString(const char* data) {
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

unsigned char Rgba::hue() const {
  float min = (float)std::min(std::min(r, g), b);
  float max = (float)std::max(std::max(r, g), b);

  if (min == max) {
    return 0;
  }

  float hue = 0.f;
  if (max == r) {
    hue = (g - b) / (max - min);

  } else if (max == g) {
    hue = 2.f + (b - r) / (max - min);

  } else {
    hue = 4.f + (r - g) / (max - min);
  }

  hue = hue * 60;
  if (hue < 0) hue = hue + 360;

  return unsigned char(hue / 360.f * 255.f);
}

Rgba Rgba::operator*(float rhs) const {
  return rhs * (*this);
}

void Rgba::fromHexString(const char* data) {
  // FFAABBFF
  // 01234567
  r = 16 * castHex(data[0]) + castHex(data[1]);
  g = 16 * castHex(data[2]) + castHex(data[3]);
  b = 16 * castHex(data[4]) + castHex(data[5]);

  switch(strlen(data)) {
    case 6:
      a = 255;
    break;
    case 8:
      a = 16 * castHex(data[6]) + castHex(data[7]);
    break;
    default:
      ERROR_AND_DIE("illegal hex string for Rgba");
    break;
  }
}

void Rgba::fromString(const char* data) {
  if(data[0] == '#') {
    fromHexString(data+1);
    return;
  } else {
    fromRgbString(data);
    return;
  }
}

std::string Rgba::toString(bool withAlpha) {
  if(withAlpha) {
    return Stringf("%u,%u,%u,%u", r, g, b, a);
  } else {
    return Stringf("%u,%u,%u", r, g, b);
  }
}

float HueToRGB(float v1, float v2, float vH) {
  if (vH < 0)
    vH += 1.f;

  if (vH > 1.f)
    vH -= 1.f;

  if ((6.f * vH) < 1.f)
    return (v1 + (v2 - v1) * 6.f * vH);

  if ((2 * vH) < 1)
    return v2;

  if ((3 * vH) < 2)
    return (v1 + (v2 - v1) * ((2.0f / 3.f) - vH) * 6.f);

  return v1;
}

Rgba operator*(float lhs, const Rgba& rhs) {
  float _r = (float)rhs.r * lhs;
  float _g = (float)rhs.g * lhs;
  float _b = (float)rhs.b * lhs;
  float _a = (float)rhs.a * lhs;

  return Rgba{ unsigned char(_r), unsigned char(_g), unsigned char(_b), unsigned char(_a) };
}

/**
 * \brief 
 * \param h Hue [0, 360]
 * \param s Saturation [0,1]
 * \param l Value [0,1]
 * \return corresponding Rgba value from HSL space
 */
Rgba Hsl(float h, float s, float l) {
  unsigned char r = 0;
  unsigned char g = 0;
  unsigned char b = 0;

  if (s == 0) {
    r = g = b = (unsigned char)(l * 255);
  } else {
    float v1, v2;
    float hue = (float)h / 360;

    v2 = (l < 0.5f) ? (l * (1.f + s)) : ((l + s) - (l * s));
    v1 = 2.f * l - v2;

    r = (unsigned char)(255.f * HueToRGB(v1, v2, hue + (1.0f / 3.f)));
    g = (unsigned char)(255.f * HueToRGB(v1, v2, hue));
    b = (unsigned char)(255.f * HueToRGB(v1, v2, hue - (1.0f / 3.f)));
  }

  return Rgba(r, g, b);
}

Rgba Hue(unsigned char hh) {
  float h = (float)hh / 255.f * 360.f;
  return Hsl(h, 1.f, .5f);
}

template<>
Rgba lerp(const Rgba& from, const Rgba& to, float t) {
  TODO("use hue to lerp color");
  float r = (float)from.r * (1 - t) + (float)to.r;
  float g = (float)from.g * (1 - t) + (float)to.g;
  float b = (float)from.b * (1 - t) + (float)to.b;
  float a = (float)from.a * (1 - t) + (float)to.a;

  return Rgba(unsigned char(r), unsigned char(g), unsigned char(b), unsigned char(a));
}
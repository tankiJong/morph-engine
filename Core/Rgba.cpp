#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"

Rgba::Rgba(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte)
	: r(redByte)
	, g(greenByte)
	, b(blueByte)
	, a(alphaByte) {}

void Rgba::setByBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte) {
	r = redByte;
	g = greenByte;
	b = greenByte;
	a = alphaByte;
}

void Rgba::setByFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha) {
	r = normalizedRed * 255;
	g = normalizedGreen * 255;
	b = normalizedBlue * 255;
	a = normalizedAlpha * 255;
}

void Rgba::getAsFloats(float & out_normalizedRed, float & out_normalizedGreen, float & out_normalizedBlue, float & out_normalizedAlpha) const {
	out_normalizedRed = 1.f / 255.f * (float)r;
	out_normalizedGreen = 1.f / 255.f * (float)g;
	out_normalizedBlue = 1.f / 255.f * (float)b;
	out_normalizedAlpha = 1.f / 255.f * (float)a;
}

void Rgba::scaleColor(float rgbScale) {
	r *= rgbScale;
	g *= rgbScale;
	b *= rgbScale;
	a *= rgbScale;

	r = clamp(r, 0, 255);
	g = clamp(g, 0, 255);
	b = clamp(b, 0, 255);
	a = clamp(a, 0, 255);
}

void Rgba::scaleOpacity(float alphaScale) {
	a *= alphaScale;
	a = clamp(a, 0, 255);
}

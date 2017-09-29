#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"


const Rgba Rgba::white = Rgba(255,255,255);

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

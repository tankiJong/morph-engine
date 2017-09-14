#pragma once
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"

struct HDC__;
typedef HDC__* HDC;
class Renderer {
public:
	void afterFrame();
	void beforeFrame();
	void drawLine(const Vector2 & start, const Vector2 & end, const Rgba& color) const;
	void setOrtho2D(const Vector2& bottomLeft, const Vector2& topRight);
	void pushMatrix();
	void popMatrix();
	void traslate(const Vector2& translation);
	void rotate2D(float degree);
	void scale(float ratio);
	void swapBuffers(HDC);
	void loadIdentity();
	void cleanScreen(const Rgba& color);
};
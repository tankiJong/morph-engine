#pragma once
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"

struct HDC__;
typedef HDC__* HDC;
class Renderer {
public:
	Renderer();
	void afterFrame();
	void beforeFrame();
	void drawLine(const Vector2 & start, const Vector2 & end, const Rgba& startColor, const Rgba& endColor, float lineThickness = 1.f) const;
	void setOrtho2D(const Vector2& bottomLeft, const Vector2& topRight);
	void pushMatrix();
	void popMatrix();
	void traslate2D(const Vector2& translation);
	void rotate2D(float degree);
	void scale2D(float ratioX, float ratioY, float ratioZ = 1.f);
	void cleanScreen(const Rgba& color);
	void loadIdentity();
protected:
	void swapBuffers(HDC);
};
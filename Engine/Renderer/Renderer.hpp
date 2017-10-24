#pragma once
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library
#include "Engine/Math/Vector2.hpp"
#include <string>
#include <map>
struct HDC__;
typedef HDC__* HDC;
class Rgba;
class Texture;
class AABB2;

// QA: forward declaring container? map, **string**
// QA: several warning in stb
class Renderer {
public:
	Renderer();
	~Renderer();
	void afterFrame();
	void beforeFrame();
	void cleanScreen(const Rgba& color);
	Texture* createOrGetTexture(const std::string& filePath);
	void drawAABB2(const AABB2& bounds, const Rgba& color);
  void drawCircle(const Vector2& center, float radius, const Rgba& color, bool filled = false);
	void drawLine(const Vector2 & start, const Vector2 & end, const Rgba& startColor, const Rgba& endColor, float lineThickness = 1.f) const;
	void drawTexturedAABB2(const AABB2& bounds, const Texture& texture,
						   const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint);
	void loadIdentity();
	void pushMatrix();
	void popMatrix();
	void rotate2D(float degree);
	void setOrtho2D(const Vector2& bottomLeft, const Vector2& topRight);
	void scale2D(float ratioX, float ratioY, float ratioZ = 1.f);
	void traslate2D(const Vector2& translation);
  void setAddtiveBlending();
  void resetAlphaBlending();
protected:
	void swapBuffers(HDC);
	std::map<std::string, Texture*>* m_textures;
};
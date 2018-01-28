#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"
#include <string>
#include <map>
#include "RenderBuffer.hpp"
#include "ShaderProgram.hpp"

struct HDC__;
struct HGLRC__;
struct HWND__;
typedef HWND__* HWND;
typedef HGLRC__* HGLRC;
typedef HDC__* HDC;
class BitmapFont;
class Rgba;
class Texture;
class AABB2;

struct Vertex_PCU {
  Vertex_PCU() {}
  Vertex_PCU(const Vector3& pos, const Rgba& col, const Vector2& uvs) : position(pos), color(col), uvs(uvs) {}
  Vertex_PCU(const Vector2& pos, const Rgba& col, const Vector2& uvs) : position(pos), color(col), uvs(uvs) {}
  Vector3 position;
  Rgba    color;
  Vector2 uvs;
};

enum DrawPrimitive {
  DRAW_UNKNOWN = -1,
  DRAW_POINTS,		// in OpenGL, for example, this becomes GL_POINTS
  DRAW_LINES,		// in OpenGL, for example, this becomes GL_LINES
  DRAW_LINE_LOOP,
  DRAW_TRIANGES,		// in OpenGL, for example, this becomes GL_TRIANGLES
  DRAW_TRIANGLE_FAN,
  DRAW_QUADS,		// in OpenGL, for example, this becomes GL_QUADS
  NUM_PRIMITIVE_TYPES
};

enum TextDrawMode {
  TEXT_DRAW_OVERRUN,
  TEXT_DRAW_SHRINK_TO_FIT,
  TEXT_DRAW_WORD_WRAP,
  NUM_TEXT_DRAW_MODE
};

class Renderer {
public:
	Renderer();
	~Renderer();
  
	void afterFrame();
	void beforeFrame();
  void bindTexutre(const Texture* texture);
	void cleanScreen(const Rgba& color);
  BitmapFont* createOrGetBitmapFont(const char* bitmapFontName, const char* path);
  BitmapFont* createOrGetBitmapFont(const char* fontNameWithPath);
	Texture* createOrGetTexture(const std::string& filePath);
  ShaderProgram* createOrGetShaderProgram(const char* nameWithPath);
  void drawAABB2(const AABB2& bounds, const Rgba& color, bool filled = true);
  void drawCircle(const Vector2& center, float radius, const Rgba& color, bool filled = false);
  void drawMeshImmediate(const Vertex_PCU* vertices, int numVerts, DrawPrimitive drawPrimitive);
	void drawLine(const Vector2 & start, const Vector2 & end, 
                const Rgba& startColor, const Rgba& endColor, 
                float lineThickness = 1.f);

	void drawTexturedAABB2(const AABB2& bounds, const Texture& texture,
                         const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint);
  void drawTexturedAABB2(const AABB2& bounds, const Texture& texture,
                         const AABB2& texCoords, const Rgba& tint = Rgba::white);

  void drawText2D(const Vector2& drawMins, const std::string& asciiText, 
                  float cellHeight, const Rgba& tint = Rgba::white,
                  float aspectScale = 1.f, const BitmapFont* font = nullptr);

  void drawText2D(const Vector2& drawMins, const std::string& asciiText,
                  float cellHeight, const BitmapFont* font = nullptr,
                  const Rgba& tint = Rgba::white, float aspectScale = 1.f);
  void drawTextInBox2D(const AABB2& bounds, const std::string& asciiText, float cellHeight, 
                       Vector2 aligns = Vector2::zero, TextDrawMode drawMode = TEXT_DRAW_OVERRUN, 
                       const BitmapFont* font = nullptr, const Rgba& tint = Rgba::white, float aspectScale = 1.f);
  bool init(HWND hwnd);

	void loadIdentity();
  void postInit();
  void pushMatrix();
	void popMatrix();
  void resetAlphaBlending();
  bool reloadShaderProgram();
  bool reloadShaderProgram(const char* nameWithPath);
	void rotate2D(float degree);
	void scale2D(float ratioX, float ratioY, float ratioZ = 1.f);
	void setOrtho2D(const Vector2& bottomLeft, const Vector2& topRight);
  void setAddtiveBlending();
	void traslate2D(const Vector2& translation);
  void useShaderProgram(ShaderProgram* program);
  static HGLRC createRealRenderContext(HDC hdc, int major, int minor);
  static HGLRC createOldRenderContext(HDC hdc);
protected:
	void swapBuffers(HDC);
	std::map<std::string, Texture*> m_textures = {};
  std::map<std::string, BitmapFont*> m_fonts = {};
  std::map<std::string, ShaderProgram*> m_shaderPrograms = {};

  // QA:...
  RenderBuffer m_tempRenderBuffer;
  ShaderProgram* m_currentShaderProgram = nullptr;
  ShaderProgram* m_defaultShaderProgram = nullptr;
  unsigned m_defaultVao;
private:

  HWND m_glWnd = nullptr;
  HDC m_hdc = nullptr;
  HGLRC m_glContext = nullptr;
};
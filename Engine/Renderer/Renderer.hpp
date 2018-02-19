#pragma once
#include <string>
#include <map>
#include <vector>

#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Primitives/vec3.hpp"

#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Math/Primitives/rect2.hpp"
#include "RenderBuffer.hpp"
#include "Camera.hpp"

struct HDC__;
struct HGLRC__;
struct HWND__;
typedef HWND__* HWND;
typedef HGLRC__* HGLRC;
typedef HDC__* HDC;
class BitmapFont;
class Rgba;
class Texture;
class aabb2;
class ShaderProgram;
class RenderBuffer;
class Sampler;
class FrameBuffer;

struct Vertex_PCU {
  Vertex_PCU() {}
  Vertex_PCU(const vec3& pos, const Rgba& col, const vec2& uvs) : position(pos), color(col), uvs(uvs) {}
  Vertex_PCU(const vec2& pos, const Rgba& col, const vec2& uvs) : position(pos), color(col), uvs(uvs) {}
  vec3 position;
  Rgba color;
  vec2 uvs;
};

enum eCompare {
  COMPARE_NEVER,       // GL_NEVER
  COMPARE_LESS,        // GL_LESS
  COMPARE_LEQUAL,      // GL_LEQUAL
  COMPARE_GREATER,     // GL_GREATER
  COMPARE_GEQUAL,      // GL_GEQUAL
  COMPARE_EQUAL,       // GL_EQUAL
  COMPARE_NOT_EQUAL,   // GL_NOTEQUAL
  COMPARE_ALWAYS,      // GL_ALWAYS
  NUM_COMPARE,
};

enum DrawPrimitive {
  DRAW_UNKNOWN = -1,
  DRAW_POINTS,
  // in OpenGL, for example, this becomes GL_POINTS
  DRAW_LINES,
  // in OpenGL, for example, this becomes GL_LINES
  DRAW_LINE_LOOP,
  DRAW_TRIANGES,
  // in OpenGL, for example, this becomes GL_TRIANGLES
  DRAW_TRIANGLE_FAN,
  DRAW_QUADS,
  // in OpenGL, for example, this becomes GL_QUADS
  NUM_PRIMITIVE_TYPES
};

enum TextDrawMode {
  TEXT_DRAW_OVERRUN,
  TEXT_DRAW_SHRINK_TO_FIT,
  TEXT_DRAW_WORD_WRAP,
  NUM_TEXT_DRAW_MODE
};

/*
 * +y
 * |
 * |    +z
 * |   /
 * |  /
 * | /
 * |/______________+x
 *
 * The coord sys i am using, left hand system. use left hand to do the cross product.
 */
class Renderer {
public:
  Renderer();
  ~Renderer();

  void afterFrame();
  void beforeFrame();
  void bindTexutre(const Texture* texture);
  void setTexture(const char* path);
  void cleanScreen(const Rgba& color);
  BitmapFont* createOrGetBitmapFont(const char* bitmapFontName, const char* path);
  BitmapFont* createOrGetBitmapFont(const char* fontNameWithPath);
  Texture* createOrGetTexture(const std::string& filePath);
  Texture* createRenderTarget(uint width, uint height,
                              eTextureFormat fmt = TEXTURE_FORMAT_RGBA8);
  inline Texture* createDepthStencilTarget(uint width, uint height) {
    return createRenderTarget(width, height, TEXTURE_FORMAT_D24S8);
  }
  ShaderProgram* createOrGetShaderProgram(const char* nameWithPath);
  void drawAABB2(const aabb2& bounds, const Rgba& color, bool filled = true);
  void drawCircle(const vec2& center, float radius, const Rgba& color, bool filled = false);
  void drawCube(const vec3& bottomCenter, const vec3& dimension, 
                const Rgba& color = Rgba::white, 
                rect2 uvTop = rect2::zero_one, rect2 uvSide = rect2::zero_one, rect2 uvBottom = rect2::zero_one);
  void drawMeshImmediate(const Vertex_PCU* vertices, size_t numVerts, DrawPrimitive drawPrimitive);
  template<size_t N>
  inline void drawMeshImmediate(const std::array<Vertex_PCU, N>& vertices, DrawPrimitive drawPrimitive) {
    this->drawMeshImmediate(vertices.data(), N, drawPrimitive);
  }
  void drawLine(const vec3& start, const vec3& end,
                const Rgba& startColor, const Rgba& endColor,
                float lineThickness = 1.f);

  void drawTexturedAABB2(const aabb2& bounds, const Texture& texture,
                         const vec2& texCoordsAtMins, const vec2& texCoordsAtMaxs, const Rgba& tint);
  void drawTexturedAABB2(const aabb2& bounds, const Texture& texture,
                         const aabb2& texCoords, const Rgba& tint = Rgba::white);

  void drawText2D(const vec2& drawMins, const std::string& asciiText,
                  float cellHeight, const Rgba& tint = Rgba::white,
                  float aspectScale = 1.f, const BitmapFont* font = nullptr);

  void drawText2D(const vec2& drawMins, const std::string& asciiText,
                  float cellHeight, const BitmapFont* font = nullptr,
                  const Rgba& tint = Rgba::white, float aspectScale = 1.f);

  void drawText2D(const vec2& drawMins, const std::vector<std::string>& asciiTexts,
                  float cellHeight, const std::vector<Rgba>& tints, const BitmapFont* font = nullptr, float aspectScale = 1.f);

  void drawTextInBox2D(const aabb2& bounds, const std::string& asciiText, float cellHeight,
                       vec2 aligns = vec2::zero, TextDrawMode drawMode = TEXT_DRAW_OVERRUN,
                       const BitmapFont* font = nullptr, const Rgba& tint = Rgba::white, float aspectScale = 1.f);
  inline Texture* getDefaultColorTarget() { return mDefaultColorTarget; }
  inline Texture* getDefaultDepthTarget() { return mDefaultDepthTarget; }
  bool init(HWND hwnd);

  void loadIdentity();
  void postInit();
  void pushMatrix();
  void popMatrix();
  void resetAlphaBlending();
  bool reloadShaderProgram();
  bool reloadShaderProgram(const char* nameWithPath);
  void rotate2D(float degree);
  void setAddtiveBlending();
  void setCamera(Camera* camera);
  void scale2D(float ratioX, float ratioY, float ratioZ = 1.f);
  void setOrtho2D(const vec2& bottomLeft, const vec2& topRight);
  void setOrtho(float width, float height, float near, float far);
  void setProjection(const mat44& projection);
  void traslate2D(const vec2& translation);
  void useShaderProgram(ShaderProgram* program);
  void clearDepth(float depth = 1.f);
  void enableDepth(eCompare compare, bool shouldWrite);
  void disableDepth();
  static HGLRC createRealRenderContext(HDC hdc, int major, int minor);
  static HGLRC createOldRenderContext(HDC hdc);
protected:
  void swapBuffers(HDC);
  bool copyFrameBuffer(FrameBuffer* dest, FrameBuffer* src);

  std::map<std::string, Texture*> mTextures = {};
  std::map<std::string, BitmapFont*> mFonts = {};
  std::map<std::string, ShaderProgram*> mShaderPrograms = {};

  // QA:...
  RenderBuffer mTempRenderBuffer;
  ShaderProgram* mCurrentShaderProgram = nullptr;
  ShaderProgram* mDefaultShaderProgram = nullptr;
  Camera* mCurrentCamera = nullptr;
  Camera* mDefaultCamera = nullptr;
  unsigned mDefaultVao;
  Sampler* mDefaultSampler = nullptr;
  const Texture* mCurrentTexture = nullptr;
  Texture* mDefaultDepthTarget = nullptr;
  Texture* mDefaultColorTarget = nullptr;
private:

  HWND mGlWnd = nullptr;
  HDC mHdc = nullptr;
  HGLRC mGlContext = nullptr;
};

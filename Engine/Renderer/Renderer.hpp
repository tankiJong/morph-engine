#pragma once
#include <string>
#include <map>
#include <vector>

#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Primitives/vec3.hpp"

#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Math/Primitives/rect2.hpp"
#include "Engine/Renderer/type.h"
#include "RenderBuffer.hpp"
#include "Camera.hpp"
#include <array>

#include "UniformBuffer.hpp"

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
class Sprite;


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
  bool applyEffect(ShaderProgram* program);
  void beforeFrame();
  void bindTexture(uint i, const Texture* texture = nullptr);
  void bindTexture(const Texture* texture = nullptr);
  void bindSampler(Sampler* sampler);
  void setTexture(const char* path);
  void setTexture(uint i, const char* path);
  void cleanScreen(const Rgba& color);
  void clearDepth(float depth = 1.f);
  bool copyTexture(Texture* from, Texture* to = nullptr);
  BitmapFont* createOrGetBitmapFont(const char* bitmapFontName, const char* path);
  BitmapFont* createOrGetBitmapFont(const char* fontNameWithPath);
  Texture* createOrGetTexture(const std::string& filePath);
  Texture* createRenderTarget(uint width, uint height,
                              eTextureFormat fmt = TEXTURE_FORMAT_RGBA8);
  inline Texture* createDepthStencilTarget(uint width, uint height) {
    return createRenderTarget(width, height, TEXTURE_FORMAT_D24S8);
  }
  ShaderProgram* createOrGetShaderProgram(const char* nameWithPath);
  void disableDepth();
  void drawAABB2(const aabb2& bounds, const Rgba& color, bool filled = true);
  void drawCircle(const vec2& center, float radius, const Rgba& color, bool filled = false);
  void drawCube(const vec3& bottomCenter, const vec3& dimension, 
                const Rgba& color = Rgba::white, 
                rect2 uvTop = rect2::zero_one, rect2 uvSide = rect2::zero_one, rect2 uvBottom = rect2::zero_one);
  void drawMeshImmediate(const Vertex_PCU* vertices, size_t numVerts, eDrawPrimitive drawPrimitive);
  template<size_t N>
  inline void drawMeshImmediate(const std::array<Vertex_PCU, N>& vertices, eDrawPrimitive drawPrimitive) {
    this->drawMeshImmediate(vertices.data(), N, drawPrimitive);
  }
  void drawLine(const vec3& start, const vec3& end,
                const Rgba& startColor, const Rgba& endColor,
                float lineThickness = 1.f);

  void drawSprite(const vec3& position, const Sprite& sprite, mat44 orientation);
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
                       vec2 aligns = vec2::zero, eTextDrawMode drawMode = TEXT_DRAW_OVERRUN,
                       const BitmapFont* font = nullptr, const Rgba& tint = Rgba::white, float aspectScale = 1.f);

  void enableDepth(eCompare compare, bool shouldWrite);
  inline Texture* getDefaultColorTarget() { return mDefaultColorTarget; }
  inline Texture* getDefaultDepthTarget() { return mDefaultDepthTarget; }
  inline Camera* getCurrentCarmara() { return mCurrentCamera; }
  bool init(HWND hwnd);

  void loadIdentity();
  void postInit();
  void pushMatrix();
  void popMatrix();
  void resetAlphaBlending();
  bool reloadShaderProgram();
  bool reloadShaderProgram(const char* nameWithPath);
  void rotate2D(float degree);
  Image screenShot();
  void setAddtiveBlending();
  void setCamera(Camera* camera);
  void scale2D(float ratioX, float ratioY, float ratioZ = 1.f);
  void setOrtho2D(const vec2& bottomLeft, const vec2& topRight);
  void setOrtho(float width, float height, float near, float far);
  void setProjection(const mat44& projection);
  void setSampler(uint i, Sampler* sampler = nullptr);
  void traslate2D(const vec2& translation);
  void updateTime(float gameDeltaSec, float sysDeltaSec);
  void useShaderProgram(ShaderProgram* program = nullptr);
  static HGLRC createRealRenderContext(HDC hdc, int major, int minor);
  static HGLRC createOldRenderContext(HDC hdc);
protected:
  void swapBuffers(HDC);
  bool copyFrameBuffer(FrameBuffer* dest, FrameBuffer* src);

  std::map<std::string, Texture*> mTextures = {};
  std::map<std::string, BitmapFont*> mFonts = {};
  std::map<std::string, ShaderProgram*> mShaderPrograms = {};

  RenderBuffer mTempRenderBuffer;
  UniformBuffer mUniformTime;
  ShaderProgram* mCurrentShaderProgram = nullptr;
  ShaderProgram* mDefaultShaderProgram = nullptr;
  Camera* mCurrentCamera = nullptr;
  owner<Camera*> mDefaultCamera = nullptr;
  owner<Camera*> mEffectCamera = nullptr;
  unsigned mDefaultVao;
  owner<Sampler*> mDefaultSampler = nullptr;
  std::array<const Texture*, 64u> mCurrentTexture{ nullptr };
  owner<Texture*> mDefaultDepthTarget = nullptr;
  owner<Texture*> mDefaultColorTarget = nullptr;
  owner<Texture*> mEffectTarget = nullptr;
  owner<Texture*> mEffectScratch = nullptr;

private:

  owner<HWND> mGlWnd = nullptr;
  owner<HDC> mHdc = nullptr;
  owner<HGLRC> mGlContext = nullptr;
};

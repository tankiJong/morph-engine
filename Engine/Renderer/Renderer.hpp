#pragma once
#include <string>
#include <map>
#include <vector>

#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Core/Rgba.hpp"

#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Math/Primitives/rect2.hpp"
#include "Engine/Renderer/type.h"
#include "RenderBuffer.hpp"
#include "Camera.hpp"
#include <array>

#include "UniformBuffer.hpp"

struct vertex_pcu_t;
class Mesh;
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
class Shader;
class Material;
class RenderTarget;
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
  void setTexture(uint i, const Texture* texture = nullptr);
  void setTexture(const Texture* texture = nullptr);
  void setSampler(const Sampler* sampler);

  void cleanColor(const Rgba& color);
  void clearDepth(float depth = 1.f);
  void cleanScreen(const Rgba& color);
  bool copyFrameBuffer(FrameBuffer* dest, FrameBuffer* src);
  bool copyTexture(Texture* from, Texture* to = nullptr);
  BitmapFont* createOrGetBitmapFont(const char* bitmapFontName, const char* path);
  BitmapFont* createOrGetBitmapFont(const char* fontNameWithPath);

  Texture* createOrGetTexture(const std::string& filePath);
  RenderTarget* createRenderTarget(uint width, uint height,
                              eTextureFormat fmt = TEXTURE_FORMAT_RGBA8);
  inline RenderTarget* createDepthStencilTarget(uint width, uint height) {
    return createRenderTarget(width, height, TEXTURE_FORMAT_D24S8);
  }
  ShaderProgram* createOrGetShaderProgram(const char* nameWithPath);
  void disableDepth();
  void disableLight();
  void disableLight(uint index);
  void drawAABB2(const aabb2& bounds, const Rgba& color, bool filled = true);
  void drawCircle(const vec2& center, float radius, const Rgba& color, bool filled = false);
  void drawCube(const vec3& bottomCenter, const vec3& dimension, 
                const Rgba& color = Rgba::white, 
                rect2 uvTop = rect2::zero_one, rect2 uvSide = rect2::zero_one, rect2 uvBottom = rect2::zero_one);
  void drawMesh(const Mesh& mesh);
  void drawMeshImmediate(const vertex_pcu_t* vertices, size_t numVerts, eDrawPrimitive drawPrimitive);
  template<size_t N>
  inline void drawMeshImmediate(const std::array<vertex_pcu_t, N>& vertices, eDrawPrimitive drawPrimitive) {
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

  inline RenderTarget* getDefaultColorTarget() { return mDefaultColorTarget; }
  inline RenderTarget* getDefaultDepthTarget() { return mDefaultDepthTarget; }
  inline Camera* getCurrentCarmara() { return mCurrentCamera; }
  bool init(HWND hwnd);

  void postInit();
  void resetAlphaBlending();
  bool reloadShaderProgram();
  bool reloadShaderProgram(const char* nameWithPath);
  Image screenShot();

  void setAmbient(const Rgba& color, float intensity);
  void setAmbient(const vec4 ambience);
  void setCamera(Camera* camera);
  void setDirectionalLight(uint index, const vec3& position, const vec3& direction,
                           float intensity = 1.f, const vec3& attenuation = vec3(0, 0, 1), const Rgba& color = Rgba::white);
  void setLight(uint index, const light_info_t& lightInfo);
  void setPointLight(uint index, const vec3& position,
                     float intensity = 1.f, const vec3& attenuation = vec3(0, 0, 1), const Rgba& color = Rgba::white);
  void setSpotLight(uint index, const vec3& position, const vec3& direction, float innerAngle, float outerAngle,
                    float intensity = 1.f, const vec3& attenuation = vec3(0, 0, 1), const Rgba& color = Rgba::white);

  void setModelMatrix(const mat44& model);
  void setMaterial(const Material* material, uint passIndex = 0);
  void setShader(const Shader* shader = nullptr, uint passIndex = 0);
  void setState(const render_state& state);
  void setSampler(uint i, const Sampler* sampler = nullptr);
  void setTexture(const char* path);
  void setTexture(uint i, const char* path);

  template<typename T>
  void setUnifrom(const char* name, const T& value);

  void setUniformBuffer(eUniformSlot slot, UniformBuffer& ubo);
  void updateTime(float gameDeltaSec, float sysDeltaSec);
  void useShaderProgram(ShaderProgram* program = nullptr);

  static HGLRC createRealRenderContext(HDC hdc, int major, int minor);
  static HGLRC createOldRenderContext(HDC hdc);
protected:

  std::map<std::string, Texture*> mTextures = {};
  std::map<std::string, BitmapFont*> mFonts = {};
  std::map<std::string, ShaderProgram*> mShaderPrograms = {};

  RenderBuffer mTempRenderBuffer;
  UniformBuffer mUniformTime;
  UniformBuffer mUniformLights;
  const Shader* mCurrentShader = nullptr;
  Shader* mDefaultShader = nullptr;
  Camera* mCurrentCamera = nullptr;
  owner<Camera*> mDefaultCamera = nullptr;
  owner<Camera*> mEffectCamera = nullptr;
  unsigned mDefaultVao = 0;
  owner<Sampler*> mDefaultSampler = nullptr;
  std::array<const Texture*, 64u> mCurrentTexture{ nullptr };
  owner<RenderTarget*> mDefaultDepthTarget = nullptr;
  owner<RenderTarget*> mDefaultColorTarget = nullptr;
  owner<RenderTarget*> mEffectTarget = nullptr;
  owner<RenderTarget*> mEffectScratch = nullptr;
private:

  owner<HWND> mGlWnd = nullptr;
  owner<HDC> mHdc = nullptr;
  owner<HGLRC> mGlContext = nullptr;
};

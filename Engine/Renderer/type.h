#pragma once
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Primitives/mat44.hpp"

// ---------------------- ENUM -----------------------------
enum eFlag {
  FLAG_FALSE = 0,
  FLAG_TRUE = 1,
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

enum eDrawPrimitive {
  DRAW_UNKNOWN = -1,
  DRAW_POINTS,
  // in OpenGL, for example, this becomes GL_POINTS
  DRAW_LINES,
  // in OpenGL, for example, this becomes GL_LINES
  DRAW_TRIANGES,
  // in OpenGL, for example, this becomes GL_TRIANGLES
  NUM_PRIMITIVE_TYPES
};

enum eTextDrawMode {
  TEXT_DRAW_OVERRUN,
  TEXT_DRAW_SHRINK_TO_FIT,
  TEXT_DRAW_WORD_WRAP,
  NUM_TEXT_DRAW_MODE
};

enum eShaderType {
  SHADER_TYPE_VERTEX,
  SHADER_TYPE_FRAGMENT,
  NUM_SHADER_TYPE,
};

enum eShaderStageState {
  STAGE_READY,
  STAGE_PREPROCESSED,
  STAGE_PREPROCESS_FAILED,
  STAGE_COMPILE_FAILED,
  NUM_STAGE_STATE,
};

enum eTextureFormat {
  TEXTURE_FORMAT_RGBA8, // default color format
  TEXTURE_FORMAT_D24S8,
};

enum eUniformSlot: uint {
  UNIFORM_TIME = 1,
  UNIFORM_CAMERA,
  UNIFORM_LIGHT,
  UNiFORM_USER_1
};

enum eTextureSlot: uint {
  TEXTURE_DIFFUSE,
  TEXTURE_NORMAL,
  TEXTURE_SPECULAR,
  TEXTURE_IMMERSIVE,
  NUM_TEXTURE_SLOT = 16,
};

enum eTextureSampleMode: uint {
  TEXTURE_SAMPLE_POINT,
  TEXTURE_SAMPLE_LINEAR,
  NUM_TEXTURE_SAMPLE_MODE,
};

enum eTextureWrapMode: uint {
  TEXTURE_WRAP_NONE,
  TEXTURE_WRAP_REPEAT,
  NUM_TEXTURE_WRAP_MODE,
};

enum eDataDeclType {
  MP_FLOAT = 0,
  MP_BYTE,
  MP_UBYTE,
  NUM_DATA_DECL_TYPE,
};

enum eCullMode {
  CULL_BACK,          // GL_BACK     glEnable(GL_CULL_FACE); glCullFace(GL_BACK); 
  CULL_FRONT,         // GL_FRONT    glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); 
  CULL_NONE,          // GL_NONE     glDisable(GL_CULL_FACE)
  NUM_CULL_MODE,
};

enum eFillMode {
  FILL_SOLID,         // GL_FILL     glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )
  FILL_WIRE,          // GL_LINE     glPolygonMode( GL_FRONT_AND_BACK, GL_LINE )
  NUM_FILL_MODE,
};

enum eWindOrder {
  WIND_CLOCKWISE,         // GL_CW       glFrontFace( GL_CW ); 
  WIND_COUNTER_CLOCKWISE, // GL_CCW      glFrontFace( GL_CCW ); 
  NUM_WIND_ORDER,
};

enum eBlendOp {
  BLEND_OP_DISABLE = -1,
  BLEND_OP_ADD, //
  BLEND_OP_SUB,
  BLEND_OP_REV_SUB,
  BLEND_OP_MIN,
  BLEND_OP_MAX,
  NUM_BLEND_OP,
};

enum eBlendFactor {
  BLEND_F_ONE,
  BLEND_F_ZERO,
  NUM_BLEND_F,
};
// ---------------------- STRUCT -----------------------------

struct uniform_time_t {
  float gameDeltaSeconds;
  float gameSeconds;
  float sysDeltaSeconds;
  float sysSeconds;
};

struct camera_t {
  mat44 projection;
  mat44 view;
};

struct light_info_t {
  vec4 color = vec4(1.f, 1.f, 1.f, 0.f);

  vec3 attenuation;
  float dotInnerAngle;

  vec3 specAttenuation;
  float dotOuterAngle;

  vec3 position;
  float directionFactor;

  vec3 direction;

  void asDirectionalLight(const vec3& position, const vec3& direction,
                          float intensity = 1.f, const vec3& attenuation = vec3(0, 0, 1),
                          const Rgba& color = Rgba::white);

  void asPointLight(const vec3& position,
                    float intensity = 1.f, const vec3& attenuation = vec3(0, 0, 1),
                    const Rgba& color = Rgba::white);

  void asSpotLight(const vec3& position, const vec3& direction, float innerAngle, float outerAngle,
                   float intensity = 1.f, const vec3& attenuation = vec3(0, 0, 1),
                   const Rgba& color = Rgba::white);
private:
  float __pad00;
};

constexpr uint NUM_MAX_LIGHTS = 8u;

  struct light_buffer_t {
  vec4 ambience;
  light_info_t lights[NUM_MAX_LIGHTS];
};

struct light_obj_t {
  float specStrength;
  float specPower;
private:
  vec2 __pad00;
};

struct render_state {
  // Raster State Control
  eCullMode cullMode = CULL_BACK;      // CULL_BACK
  eFillMode fillMode = FILL_SOLID;      // FILL_SOLID
  eWindOrder frontFace = WIND_COUNTER_CLOCKWISE;    // WIND_COUNTER_CLOCKWISE

                              // Depth State Control
  eCompare depthMode = COMPARE_LESS;   // COMPARE_LESS
  eFlag isWriteDepth = FLAG_TRUE;         // true

                              // Blend
  eBlendOp colorBlendOp = BLEND_OP_ADD;          // COMPARE_ADD
  eBlendFactor colorSrcFactor = BLEND_F_ONE;    // BLEND_ONE
  eBlendFactor colorDstFactor = BLEND_F_ZERO;    // BLEND_ZERO

  eBlendOp alphaBlendOp = BLEND_OP_ADD;          // COMPARE_ADD
  eBlendFactor alphaSrcFactor = BLEND_F_ONE;    // BLEND_ONE
  eBlendFactor alphaDstFactor = BLEND_F_ZERO;    // BLEND_ONE
};
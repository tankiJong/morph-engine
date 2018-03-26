#pragma once
#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Core/Rgba.hpp"
// ---------------------- ENUM -----------------------------
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
  DRAW_LINE_LOOP,
  DRAW_TRIANGES,
  // in OpenGL, for example, this becomes GL_TRIANGLES
  DRAW_TRIANGLE_FAN,
  DRAW_QUADS,
  // in OpenGL, for example, this becomes GL_QUADS
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

enum eUniformUnit {
  UNIFORM_TIME = 1,
  UNIFORM_TRANSFORM,
  UNIFORM_CAMERA,
  UNIFORM_DELTATIME,
};
// ---------------------- STRUCT -----------------------------

struct uniform_time_t {
  float gameDeltaSeconds;
  float gameSeconds;
  float sysDeltaSeconds;
  float sysSeconds;
};

struct Vertex_PCU {
  Vertex_PCU() {}
  Vertex_PCU(const vec3& pos, const Rgba& col, const vec2& uvs) : position(pos), color(col), uvs(uvs) {}
  Vertex_PCU(const vec2& pos, const Rgba& col, const vec2& uvs) : position(pos), color(col), uvs(uvs) {}
  vec3 position;
  Rgba color;
  vec2 uvs;
};
#include "ShaderStage.hpp"
#include "Engine/Tool/Parser.hpp"

static auto eol = many_of("\n\r");
static auto singleLineCommentContent = except_of("\n\r");
static auto singleLineComment = "//"_P < singleLineCommentContent > eol;

static auto mutiLineCommentContent = many<std::string>(
  except_of("*/") | "/"_P | "/*"_P | "*"_P,
  "",
  [](std::string a, std::string b) { return a.append(b); });
static auto mutiLineComment = R"(\*)"_P < mutiLineCommentContent > "*/"_P;

static auto commentParser = mutiLineComment | singleLineComment;
bool xx() {
  static const char* defaultVertexShader
    = R"(// hahaha
/*
#version 420 core
  */
#version 420 core
uniform mat4 PROJECTION; 

in vec3 POSITION;
in vec4 COLOR;       // NEW - GLSL will use a Vector4 for this; 
in vec2 UV;         

out vec2 passUV; 
out vec4 passColor;  // NEW - to use it in the pixel stage, we must pass it.

void main() 
{
   vec4 local_pos = vec4( POSITION, 1 ); 
   vec4 clip_pos = PROJECTION * local_pos; 

   passColor = COLOR; // pass it on. 
   passUV = UV; 
   gl_Position = clip_pos; 
})";

  auto t = commentParser(std::string(defaultVertexShader));

  return t.has_value();
}

bool aa = xx();
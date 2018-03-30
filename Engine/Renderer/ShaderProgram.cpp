#include "ShaderProgram.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/File/Utils.hpp"
#include "glFunctions.hpp"
#include "Engine/Core/StringUtils.hpp"

static const char* defaultVertexShader 
= R"(#version 420 core
uniform mat4 PROJECTION; 
uniform mat4 VIEW; 
in vec3 POSITION;
in vec4 COLOR;       // NEW - GLSL will use a Vector4 for this; 
in vec2 UV;         

out vec2 passUV; 
out vec4 passColor;  // NEW - to use it in the pixel stage, we must pass it.
out vec3 passPos;
out vec4 ViewMat;

void main() 
{
   vec4 local_pos = vec4( POSITION, 1 ); 
   vec4 clip_pos = PROJECTION * VIEW * local_pos; 

   passColor = COLOR; // pass it on. 
   passUV = UV; 
   passPos = clip_pos.xyz;
   ViewMat = PROJECTION * VIEW * vec4(0);
   gl_Position = clip_pos; 
})";

static const char* defaultFragmentShader = 
R"(#version 420 core
layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec4 passColor; // NEW, passed color
in vec2 passUV; 
in vec3 passPos;
out vec4 outColor; 
  
void main() 
{
   vec4 diffuse = texture( gTexDiffuse, passUV );
   
   // multiply is component-wise
   // so this gets (diff.x * passColor.x, ..., diff.w * passColor.w)
   outColor = diffuse * passColor;  
})";

static const char* invalidFragmentShader =
R"(#version 420 core
out vec4 outColor; 
void main( void )
{
   outColor = vec4( 1, 0, 1, 1 ); 
})";

void addDefinesToStage(ShaderStage& stage, const char* defineArgs) {
  if (defineArgs == nullptr) return;
  auto defs = split(defineArgs, ";");
  for (auto& def : defs) {
    uint sp = def.find('=');

    if (sp != std::string::npos) {
      stage.define(std::string(def.begin(), def.begin() + sp), std::string(def.begin() + sp + 1, def.end()));
    } else {
      stage.define(def);
    }
  }
}
bool ShaderProgram::fromFile(const char* relativePath, const char* defineArgs) {
  addDefinesToStage(stages[SHADER_TYPE_VERTEX], defineArgs);
  addDefinesToStage(stages[SHADER_TYPE_FRAGMENT], defineArgs);
  if(strcmp(relativePath, "@default") == 0) {
    stages[SHADER_TYPE_VERTEX].setFromString(SHADER_TYPE_VERTEX, defaultVertexShader);
    stages[SHADER_TYPE_FRAGMENT].setFromString(SHADER_TYPE_FRAGMENT, defaultFragmentShader);

  } else if (strcmp(relativePath, "@invalid") == 0) {
    stages[SHADER_TYPE_VERTEX].setFromString(SHADER_TYPE_VERTEX, defaultVertexShader);
    stages[SHADER_TYPE_FRAGMENT].setFromString(SHADER_TYPE_FRAGMENT, invalidFragmentShader);

  } else {
    std::string vsFile = relativePath;
    vsFile += ".vs";

    std::string fsFile = relativePath;
    fsFile += ".fs";

    stages[SHADER_TYPE_VERTEX].setFromFile(SHADER_TYPE_VERTEX, vsFile.c_str());
    stages[SHADER_TYPE_FRAGMENT].setFromFile(SHADER_TYPE_FRAGMENT, fsFile.c_str());

  }


  for(auto& stage: stages) {
    stage.compile();
  }
  GLuint vs = stages[SHADER_TYPE_VERTEX].handle();
  GLuint fs = stages[SHADER_TYPE_FRAGMENT].handle();

  if (vs == NULL || fs == NULL) {
    return false;
  }
  programHandle = createAndLinkProgram(vs, fs, programHandle);

  TODO("move to shaderstage");
  glDeleteShader(vs);
  glDeleteShader(fs);
  GL_CHECK_ERROR();

  return (programHandle != NULL);
}

GLuint ShaderProgram::createAndLinkProgram(GLuint vs, GLuint fs, GLuint handle) {
  // credate the program handle - how you will reference
  // this program within OpenGL, like a texture handle
  GLuint programId = handle;
  if(programId == 0) {
    programId = glCreateProgram();
  }
  EXPECTS(programId != 0);

  // Attach the shaders you want to use
  glAttachShader(programId, vs);
  glAttachShader(programId, fs);

  GL_CHECK_ERROR();
  // Link the program (create the GPU program)
  glLinkProgram(programId);
  GL_CHECK_ERROR();

  // Check for link errors - usually a result
  // of incompatibility between stages.
  GLint linkStatus;
  glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
  GL_CHECK_ERROR();

  if (linkStatus == GL_FALSE) {
    logProgramError(programId);
    glDeleteProgram(programId);
    programId = 0;
  }

  // no longer need the shaders, you can detach them if you want
  // (not necessary)
  glDetachShader(programId, vs);
  glDetachShader(programId, fs);
  GL_CHECK_ERROR();

  return programId;

}

void ShaderProgram::logProgramError(GLuint programId) {
  // figure out how large the buffer needs to be
  GLint length = 0;
  glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);

  // make a buffer, and copy the log to it. 
  char* buffer = new char[length + 1];
  glGetProgramInfoLog(programId, length, &length, buffer);

  // Print it out (may want to do some additional formatting)
  buffer[length] = NULL;
  DebuggerPrintf("class", buffer);

  ERROR_RECOVERABLE(Stringf("Linking Shader error, %s", buffer));

//  DEBUGBREAK;

  // free up the memory we used. 
  delete []buffer;

}

//GLuint ShaderProgram::loadShader(const char* rawShader, GLenum type, const char* defineArgs) {
//  
//  EXPECTS(rawShader != nullptr);
//
//  // Create a shader
//  GLuint shaderId = glCreateShader(type);
//  EXPECTS(shaderId != NULL);
//
//  // Bind source to it, and compile
//  // You can add multiple strings to a shader – they will 
//  // be concatenated together to form the actual source object.
//  GLint shaderLength = (GLint)strlen(rawShader);
//
//  Blob shaderStr = injectDefine(rawShader, shaderLength, defineArgs);
//  shaderLength = (GLint)shaderStr.size();
//
//  glShaderSource(shaderId, 1, (GLchar**)&shaderStr, &shaderLength);
//  glCompileShader(shaderId);
//
//  // Check status
//  GLint status;
//  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
//  if (status == GL_FALSE) {
//    logShaderError(shaderId); // function we write
//    glDeleteShader(shaderId);
//    shaderId = NULL;
//  }
//
//  return shaderId;
//
//}
//
//GLuint ShaderProgram::loadShader(const Blob& rawShader, GLenum type, const char* defineArgs) {
//
//  // Create a shader
//  GLuint shaderId = glCreateShader(type);
//  EXPECTS(shaderId != NULL);
//
//  // Bind source to it, and compile
//  // You can add multiple strings to a shader – they will 
//  // be concatenated together to form the actual source object.
//
//  Blob shaderStr = injectDefine(rawShader, rawShader.size(), defineArgs);
//  GLint shaderLength = (GLint)shaderStr.size();
//
//  glShaderSource(shaderId, 1, (GLchar**)&shaderStr, &shaderLength);
//  glCompileShader(shaderId);
//
//  // Check status
//  GLint status;
//  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
//  if (status == GL_FALSE) {
//    logShaderError(shaderId); // function we write
//    glDeleteShader(shaderId);
//    shaderId = NULL;
//  }
//
//  return shaderId;
//
//}
//
//Blob ShaderProgram::injectDefine(const char* buffer, size_t size, const char* defines) {
//  if (defines == nullptr) return Blob(buffer, size);
//  auto defs = split(defines, ";");
//
//  static constexpr size_t injectedTokenLen = 10; // length of "#define  \n"
//
//  size_t blockSize 
//    = strlen(defines)                 // token to be injected
//    + defs.size() * injectedTokenLen  // additional token in each line
//    + size;                   // original file length
//  char* injected = (char*)malloc(blockSize);
//      
//  size_t p = 0;
//
//  int offect = 0;
//  while (buffer[p] != '\n') {
//    injected[offect++] =buffer[p++];
//
//    if (buffer[p] == '\0') ERROR_AND_DIE("illegal shader file");
//  }
//
//  injected[offect++] =buffer[p++]; // put in \n
//
//
//  for(auto& def: defs) {
//    injected[offect] = '\0';
//    strcat_s(injected, blockSize, "#define ");
//    offect += 8; // length of "#define "
//
//    const char* q = def.c_str();
//
//    while(*q != '=' && *q != '\0') {
//      injected[offect++] = *(q++);
//    }
//
//    if(*(q++) == '=') {
//      injected[offect++] = ' ';
//      while(*q != '\0') {
//        injected[offect++] = (*q++);
//      }
//    }
//
//    injected[offect++] = '\n';
//  }
//
//  while(buffer[p] != '\0') {
//    injected[offect++] =buffer[p++];
//  }
//
//  injected[offect++] = '\0';
//
//  return Blob(injected, blockSize);
//}

#include "ShaderProgram.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/File/FileUtils.hpp"
#include "glFunctions.hpp"
#include "Engine/Core/StringUtils.hpp"

static const char* defaultVertexShader 
= R"(#version 420 core
in vec3 POSITION;
void main( void )
{
   gl_Position = vec4( POSITION, 1 ); 
})";

static const char* defaultFragmentShader = 
R"(#version 420 core
out vec4 outColor; 
void main( void )
{
   outColor = vec4( 1, 0, 0, 1 ); 
})";

static const char* invalidFragmentShader =
R"(#version 420 core
out vec4 outColor; 
void main( void )
{
   outColor = vec4( 1, 0, 1, 1 ); 
})";

bool ShaderProgram::fromFile(const char* relativePath, const char* defineArgs) {
  if(strcmp(relativePath, "@default") == 0) {
    GLuint vs = loadShader(defaultVertexShader, GL_VERTEX_SHADER, defineArgs);
    GLuint fs = loadShader(defaultFragmentShader, GL_FRAGMENT_SHADER, defineArgs);
    if(vs == NULL || fs == NULL) {
      return false;
    }
    programHandle = createAndLinkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return (programHandle != NULL);
  }

  if (strcmp(relativePath, "@invalid") == 0) {
    GLuint vs = loadShader(defaultVertexShader, GL_VERTEX_SHADER, defineArgs);
    GLuint fs = loadShader(invalidFragmentShader, GL_FRAGMENT_SHADER, defineArgs);

    programHandle = createAndLinkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return (programHandle != NULL);
  }

  std::string vsFile = relativePath;
  vsFile += ".vs";

  std::string fsFile = relativePath;
  fsFile += ".fs";

  Blob vsData = fileToBuffer(vsFile.c_str());
  GLuint vs = loadShader(vsData, GL_VERTEX_SHADER, defineArgs);
  Blob fsData = fileToBuffer(fsFile.c_str());
  GLuint fs = loadShader(fsData, GL_FRAGMENT_SHADER, defineArgs);

  if(vs == NULL || fs == NULL) {
    return false;
  }
  programHandle = createAndLinkProgram(vs, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  return (programHandle != NULL);
}

GLuint ShaderProgram::createAndLinkProgram(GLuint vs, GLuint fs) {
  // credate the program handle - how you will reference
  // this program within OpenGL, like a texture handle
  GLuint programId = glCreateProgram();
  EXPECTS(programId != 0);

  // Attach the shaders you want to use
  glAttachShader(programId, vs);
  glAttachShader(programId, fs);

  // Link the program (create the GPU program)
  glLinkProgram(programId);

  // Check for link errors - usually a result
  // of incompatibility between stages.
  GLint linkStatus;
  glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);

  if (linkStatus == GL_FALSE) {
    logShaderError(programId);
    glDeleteProgram(programId);
    programId = 0;
  }

  // no longer need the shaders, you can detach them if you want
  // (not necessary)
  glDetachShader(programId, vs);
  glDetachShader(programId, fs);

  return programId;

}

void ShaderProgram::logShaderError(GLuint shaderId) {
  // figure out how large the buffer needs to be
  GLint length = 0;
  glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);

  // make a buffer, and copy the log to it. 
  char* buffer = new char[length + 1];
  glGetShaderInfoLog(shaderId, length, &length, buffer);

  // Print it out (may want to do some additional formatting)
  buffer[length] = NULL;
  DebuggerPrintf("class", buffer);

  ERROR_RECOVERABLE(Stringf("Compile Shader error, %s", buffer));

//  DEBUGBREAK;

  // free up the memory we used. 
  delete []buffer;

}

GLuint ShaderProgram::loadShader(const char* rawShader, GLenum type, const char* defineArgs) {
  
  EXPECTS(rawShader != nullptr);

  // Create a shader
  GLuint shaderId = glCreateShader(type);
  EXPECTS(shaderId != NULL);

  // Bind source to it, and compile
  // You can add multiple strings to a shader – they will 
  // be concatenated together to form the actual source object.
  GLint shaderLength = (GLint)strlen(rawShader);

  Blob shaderStr = injectDefine(rawShader, shaderLength, defineArgs);
  shaderLength = (GLint)shaderStr.size;

  glShaderSource(shaderId, 1, (GLchar**)&shaderStr, &shaderLength);
  glCompileShader(shaderId);

  // Check status
  GLint status;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    logShaderError(shaderId); // function we write
    glDeleteShader(shaderId);
    shaderId = NULL;
  }

  return shaderId;

}

GLuint ShaderProgram::loadShader(const Blob& rawShader, GLenum type, const char* defineArgs) {

  // Create a shader
  GLuint shaderId = glCreateShader(type);
  EXPECTS(shaderId != NULL);

  // Bind source to it, and compile
  // You can add multiple strings to a shader – they will 
  // be concatenated together to form the actual source object.

  Blob shaderStr = injectDefine(rawShader, rawShader.size, defineArgs);
  GLint shaderLength = (GLint)shaderStr.size;

  glShaderSource(shaderId, 1, (GLchar**)shaderStr, &shaderLength);
  glCompileShader(shaderId);

  // Check status
  GLint status;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    logShaderError(shaderId); // function we write
    glDeleteShader(shaderId);
    shaderId = NULL;
  }

  return shaderId;

}

Blob ShaderProgram::injectDefine(const char* buffer, size_t size, const char* defines) {
  if (defines == nullptr) return Blob(buffer, size);
  auto defs = split(defines, ";");

  static constexpr size_t injectedTokenLen = 10; // length of "#define  \n"

  size_t blockSize 
    = strlen(defines)                 // token to be injected
    + defs.size() * injectedTokenLen  // additional token in each line
    + size;                   // original file length
  char* injected = (char*)malloc(blockSize);
      
  size_t p = 0;

  int offect = 0;
  while (buffer[p] != '\n') {
    injected[offect++] =buffer[p++];

    if (buffer[p] == '\0') ERROR_AND_DIE("illegal shader file");
  }

  injected[offect++] =buffer[p++]; // put in \n


  for(auto& def: defs) {
    injected[offect] = '\0';
    strcat_s(injected, sizeof(injected), "#define "); 
    offect += 8; // length of "#define "

    const char* q = def.c_str();

    while(*q != '=' && *q != '\0') {
      injected[offect++] = *(q++);
    }

    if(*(q++) == '=') {
      injected[offect++] = ' ';
      while(*q != '\0') {
        injected[offect++] = (*q++);
      }
    }

    injected[offect++] = '\n';
  }

  while(buffer[p] != '\0') {
    injected[offect++] =buffer[p++];
  }

  injected[offect++] = '\0';

  return Blob(injected, blockSize);
}

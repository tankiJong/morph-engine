#include "glFunctions.hpp"

extern HMODULE gGlLibrary = nullptr;

template <typename T>
bool wglGetTypedProcAddress(T *out, char const *name) {
  // Grab the function from the currently bound render contect
  // most opengl 2.0+ features will be found here
  *out = (T)wglGetProcAddress(name);

  if ((*out) == nullptr) {
    // if it is not part of wgl (the device), then attempt to get it from the GLL library
    // (most OpenGL functions come from here)
    *out = (T)GetProcAddress(gGlLibrary, name);
  }

  return (*out != nullptr);
}

#define GL_BIND_FUNCTION(f)      wglGetTypedProcAddress( &f, #f )

PFNWGLGETEXTENSIONSSTRINGARBPROC  wglGetExtensionsStringARB  = nullptr;
PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB    = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

PFNGLATTACHSHADERPROC            glAttachShader            = nullptr;
PFNGLBINDBUFFERPROC              glBindBuffer              = nullptr;
PFNGLBINDVERTEXARRAYPROC         glBindVertexArray         = nullptr;
PFNGLBUFFERDATAPROC              glBufferData              = nullptr;
PFNGLCLEARPROC                   glClear                   = nullptr;
PFNGLCLEARCOLORPROC              glClearColor              = nullptr;
PFNGLCREATESHADERPROC            glCreateShader            = nullptr;
PFNGLCREATEPROGRAMPROC           glCreateProgram           = nullptr;
PFNGLCOMPILESHADERPROC           glCompileShader           = nullptr;
PFNGLDELETEBUFFERSPROC           glDeleteBuffers           = nullptr;
PFNGLDELETEPROGRAMPROC           glDeleteProgram           = nullptr;
PFNGLDELETESAMPLERSPROC          glDeleteSamplers          = nullptr;
PFNGLDELETESHADERPROC            glDeleteShader            = nullptr;
PFNGLDETACHSHADERPROC            glDetachShader            = nullptr;
PFNGLDRAWARRAYSPROC              glDrawArrays              = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLGENBUFFERSPROC              glGenBuffers              = nullptr;
PFNGLGENSAMPLERSPROC             glGenSamplers             = nullptr;
PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays         = nullptr;
PFNGLGETATTRIBLOCATIONPROC       glGetAttribLocation       = nullptr;
PFNGLGETPROGRAMIVPROC            glGetProgramiv            = nullptr;
PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog        = nullptr;
PFNGLGETSHADERIVPROC             glGetShaderiv             = nullptr;
PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation      = nullptr;
PFNGLLINKPROGRAMPROC             glLinkProgram             = nullptr;
PFNGLSAMPLERPARAMETERIPROC       glSamplerParameteri       = nullptr;
PFNGLSHADERSOURCEPROC            glShaderSource            = nullptr;
PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv        = nullptr;
PFNGLUSEPROGRAMPROC              glUseProgram              = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer     = nullptr;

void bindNewWGLFunctions() {
  GL_BIND_FUNCTION(wglGetExtensionsStringARB);
  GL_BIND_FUNCTION(wglChoosePixelFormatARB);
  GL_BIND_FUNCTION(wglCreateContextAttribsARB);
}

void bindGLFunctions() {
  wglGetTypedProcAddress( &glAttachShader, "glAttachShader" );
  GL_BIND_FUNCTION(glBindBuffer);
  GL_BIND_FUNCTION(glBindVertexArray);
  GL_BIND_FUNCTION(glBufferData);
  GL_BIND_FUNCTION(glClear);
  GL_BIND_FUNCTION(glCreateShader);
  GL_BIND_FUNCTION(glCreateProgram);
  GL_BIND_FUNCTION(glCompileShader);
  GL_BIND_FUNCTION(glDeleteBuffers);
  GL_BIND_FUNCTION(glDeleteProgram);
  GL_BIND_FUNCTION(glDeleteSamplers);
  GL_BIND_FUNCTION(glDeleteShader);
  GL_BIND_FUNCTION(glDetachShader);
  GL_BIND_FUNCTION(glDrawArrays);
  GL_BIND_FUNCTION(glEnableVertexAttribArray);
  GL_BIND_FUNCTION(glGenBuffers);
  GL_BIND_FUNCTION(glGenSamplers);
  GL_BIND_FUNCTION(glGenVertexArrays);
  GL_BIND_FUNCTION(glGetAttribLocation);
  GL_BIND_FUNCTION(glGetProgramiv);
  GL_BIND_FUNCTION(glGetShaderInfoLog);
  GL_BIND_FUNCTION(glGetShaderiv);
  GL_BIND_FUNCTION(glGetUniformLocation);
  GL_BIND_FUNCTION(glLinkProgram);
  GL_BIND_FUNCTION(glSamplerParameteri);
  GL_BIND_FUNCTION(glShaderSource);
  GL_BIND_FUNCTION(glUniformMatrix4fv);
  GL_BIND_FUNCTION(glUseProgram);
  GL_BIND_FUNCTION(glVertexAttribPointer);
  GL_BIND_FUNCTION(glClearColor);
}

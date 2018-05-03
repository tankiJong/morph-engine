#include "glFunctions.hpp"
#include "type.h"

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

PFNGLACTIVETEXTUREPROC                               glActiveTexture                               = nullptr;
PFNGLBINDIMAGETEXTUREPROC                            glBindImageTexture                            = nullptr;
PFNGLBINDTEXTUREPROC                                 glBindTexture                                 = nullptr;
PFNGLCOMPRESSEDTEXIMAGE1DPROC                        glCompressedTexImage1D                        = nullptr;
PFNGLCOMPRESSEDTEXIMAGE2DPROC                        glCompressedTexImage2D                        = nullptr;
PFNGLCOMPRESSEDTEXIMAGE3DPROC                        glCompressedTexImage3D                        = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC                     glCompressedTexSubImage1D                     = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC                     glCompressedTexSubImage2D                     = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC                     glCompressedTexSubImage3D                     = nullptr;
PFNGLCOPYTEXIMAGE1DPROC                              glCopyTexImage1D                              = nullptr;
PFNGLCOPYTEXIMAGE2DPROC                              glCopyTexImage2D                              = nullptr;
PFNGLCOPYTEXSUBIMAGE1DPROC                           glCopyTexSubImage1D                           = nullptr;
PFNGLCOPYTEXSUBIMAGE2DPROC                           glCopyTexSubImage2D                           = nullptr;
PFNGLCOPYTEXSUBIMAGE3DPROC                           glCopyTexSubImage3D                           = nullptr;
PFNGLDELETETEXTURESPROC                              glDeleteTextures                              = nullptr;
PFNGLGENTEXTURESPROC                                 glGenTextures                                 = nullptr;
PFNGLGETCOMPRESSEDTEXIMAGEPROC                       glGetCompressedTexImage                       = nullptr;
PFNGLGETTEXIMAGEPROC                                 glGetTexImage                                 = nullptr;
PFNGLGETTEXLEVELPARAMETERFVPROC                      glGetTexLevelParameterfv                      = nullptr;
PFNGLGETTEXLEVELPARAMETERIVPROC                      glGetTexLevelParameteriv                      = nullptr;
PFNGLGETTEXPARAMETERFVPROC                           glGetTexParameterfv                           = nullptr;
PFNGLGETTEXPARAMETERIVPROC                           glGetTexParameteriv                           = nullptr;
PFNGLISTEXTUREPROC                                   glIsTexture                                   = nullptr;
PFNGLTEXBUFFERPROC                                   glTexBuffer                                   = nullptr;
PFNGLTEXIMAGE1DPROC                                  glTexImage1D                                  = nullptr;
PFNGLTEXIMAGE2DPROC                                  glTexImage2D                                  = nullptr;
PFNGLTEXIMAGE2DMULTISAMPLEPROC                       glTexImage2DMultisample                       = nullptr;
PFNGLTEXIMAGE3DPROC                                  glTexImage3D                                  = nullptr;
PFNGLTEXIMAGE3DMULTISAMPLEPROC                       glTexImage3DMultisample                       = nullptr;
PFNGLTEXPARAMETERIIVPROC                             glTexParameterIiv                             = nullptr;
PFNGLTEXPARAMETERIUIVPROC                            glTexParameterIuiv                            = nullptr;
PFNGLTEXPARAMETERFPROC                               glTexParameterf                               = nullptr;
PFNGLTEXPARAMETERFVPROC                              glTexParameterfv                              = nullptr;
PFNGLTEXPARAMETERIPROC                               glTexParameteri                               = nullptr;
PFNGLTEXPARAMETERIVPROC                              glTexParameteriv                              = nullptr;
PFNGLTEXSTORAGE1DPROC                                glTexStorage1D                                = nullptr;
PFNGLTEXSTORAGE2DPROC                                glTexStorage2D                                = nullptr;
PFNGLTEXSTORAGE3DPROC                                glTexStorage3D                                = nullptr;
PFNGLTEXSUBIMAGE1DPROC                               glTexSubImage1D                               = nullptr;
PFNGLTEXSUBIMAGE2DPROC                               glTexSubImage2D                               = nullptr;
PFNGLTEXSUBIMAGE3DPROC                               glTexSubImage3D                               = nullptr;
PFNGLCLEARPROC                                       glClear                                       = nullptr;
PFNGLCLEARBUFFERFIPROC                               glClearBufferfi                               = nullptr;
PFNGLCLEARBUFFERFVPROC                               glClearBufferfv                               = nullptr;
PFNGLCLEARBUFFERIVPROC                               glClearBufferiv                               = nullptr;
PFNGLCLEARBUFFERUIVPROC                              glClearBufferuiv                              = nullptr;
PFNGLCLEARCOLORPROC                                  glClearColor                                  = nullptr;
PFNGLCLEARDEPTHPROC                                  glClearDepth                                  = nullptr;
PFNGLCLEARDEPTHFPROC                                 glClearDepthf                                 = nullptr;
PFNGLCLEARSTENCILPROC                                glClearStencil                                = nullptr;
PFNGLDRAWBUFFERPROC                                  glDrawBuffer                                  = nullptr;
PFNGLFINISHPROC                                      glFinish                                      = nullptr;
PFNGLFLUSHPROC                                       glFlush                                       = nullptr;
PFNGLREADBUFFERPROC                                  glReadBuffer                                  = nullptr;
PFNGLREADPIXELSPROC                                  glReadPixels                                  = nullptr;
PFNGLBINDFRAMEBUFFERPROC                             glBindFramebuffer                             = nullptr;
PFNGLBINDRENDERBUFFERPROC                            glBindRenderbuffer                            = nullptr;
PFNGLBLITFRAMEBUFFERPROC                             glBlitFramebuffer                             = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC                      glCheckFramebufferStatus                      = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC                          glDeleteFramebuffers                          = nullptr;
PFNGLDELETERENDERBUFFERSPROC                         glDeleteRenderbuffers                         = nullptr;
PFNGLDRAWBUFFERSPROC                                 glDrawBuffers                                 = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC                     glFramebufferRenderbuffer                     = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC                          glFramebufferTexture                          = nullptr;
PFNGLFRAMEBUFFERTEXTURELAYERPROC                     glFramebufferTextureLayer                     = nullptr;
PFNGLGENFRAMEBUFFERSPROC                             glGenFramebuffers                             = nullptr;
PFNGLGENRENDERBUFFERSPROC                            glGenRenderbuffers                            = nullptr;
PFNGLGENERATEMIPMAPPROC                              glGenerateMipmap                              = nullptr;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC         glGetFramebufferAttachmentParameteriv         = nullptr;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC    glGetNamedFramebufferAttachmentParameteriv    = nullptr;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC             glGetNamedRenderbufferParameteriv             = nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVPROC                  glGetRenderbufferParameteriv                  = nullptr;
PFNGLISFRAMEBUFFERPROC                               glIsFramebuffer                               = nullptr;
PFNGLISRENDERBUFFERPROC                              glIsRenderbuffer                              = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC                         glRenderbufferStorage                         = nullptr;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC              glRenderbufferStorageMultisample              = nullptr;
PFNGLSAMPLEMASKIPROC                                 glSampleMaski                                 = nullptr;
PFNGLATTACHSHADERPROC                                glAttachShader                                = nullptr;
PFNGLBINDATTRIBLOCATIONPROC                          glBindAttribLocation                          = nullptr;
PFNGLBINDFRAGDATALOCATIONPROC                        glBindFragDataLocation                        = nullptr;
PFNGLBINDFRAGDATALOCATIONINDEXEDPROC                 glBindFragDataLocationIndexed                 = nullptr;
PFNGLCOMPILESHADERPROC                               glCompileShader                               = nullptr;
PFNGLCREATEPROGRAMPROC                               glCreateProgram                               = nullptr;
PFNGLCREATESHADERPROC                                glCreateShader                                = nullptr;
PFNGLCREATESHADERPROGRAMVPROC                        glCreateShaderProgramv                        = nullptr;
PFNGLDELETEPROGRAMPROC                               glDeleteProgram                               = nullptr;
PFNGLDELETESHADERPROC                                glDeleteShader                                = nullptr;
PFNGLDETACHSHADERPROC                                glDetachShader                                = nullptr;
PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC              glGetActiveAtomicCounterBufferiv              = nullptr;
PFNGLGETACTIVEATTRIBPROC                             glGetActiveAttrib                             = nullptr;
PFNGLGETACTIVESUBROUTINENAMEPROC                     glGetActiveSubroutineName                     = nullptr;
PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC                glGetActiveSubroutineUniformiv                = nullptr;
PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC              glGetActiveSubroutineUniformName              = nullptr;
PFNGLGETACTIVEUNIFORMPROC                            glGetActiveUniform                            = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC                     glGetActiveUniformBlockiv                     = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC                   glGetActiveUniformBlockName                   = nullptr;
PFNGLGETACTIVEUNIFORMNAMEPROC                        glGetActiveUniformName                        = nullptr;
PFNGLGETACTIVEUNIFORMSIVPROC                         glGetActiveUniformsiv                         = nullptr;
PFNGLGETATTACHEDSHADERSPROC                          glGetAttachedShaders                          = nullptr;
PFNGLGETATTRIBLOCATIONPROC                           glGetAttribLocation                           = nullptr;
PFNGLGETFRAGDATAINDEXPROC                            glGetFragDataIndex                            = nullptr;
PFNGLGETFRAGDATALOCATIONPROC                         glGetFragDataLocation                         = nullptr;
PFNGLGETPROGRAMIVPROC                                glGetProgramiv                                = nullptr;
PFNGLGETPROGRAMBINARYPROC                            glGetProgramBinary                            = nullptr;
PFNGLGETPROGRAMINFOLOGPROC                           glGetProgramInfoLog                           = nullptr;
PFNGLGETPROGRAMSTAGEIVPROC                           glGetProgramStageiv                           = nullptr;
PFNGLGETSHADERIVPROC                                 glGetShaderiv                                 = nullptr;
PFNGLGETSHADERINFOLOGPROC                            glGetShaderInfoLog                            = nullptr;
PFNGLGETSHADERPRECISIONFORMATPROC                    glGetShaderPrecisionFormat                    = nullptr;
PFNGLGETSHADERSOURCEPROC                             glGetShaderSource                             = nullptr;
PFNGLGETSUBROUTINEINDEXPROC                          glGetSubroutineIndex                          = nullptr;
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC                glGetSubroutineUniformLocation                = nullptr;
PFNGLGETUNIFORMDVPROC                                glGetUniformdv                                = nullptr;
PFNGLGETUNIFORMFVPROC                                glGetUniformfv                                = nullptr;
PFNGLGETUNIFORMIVPROC                                glGetUniformiv                                = nullptr;
PFNGLGETUNIFORMUIVPROC                               glGetUniformuiv                               = nullptr;
PFNGLGETNUNIFORMDVPROC                               glGetnUniformdv                               = nullptr;
PFNGLGETNUNIFORMFVPROC                               glGetnUniformfv                               = nullptr;
PFNGLGETNUNIFORMIVPROC                               glGetnUniformiv                               = nullptr;
PFNGLGETNUNIFORMUIVPROC                              glGetnUniformuiv                              = nullptr;
PFNGLGETUNIFORMBLOCKINDEXPROC                        glGetUniformBlockIndex                        = nullptr;
PFNGLGETUNIFORMINDICESPROC                           glGetUniformIndices                           = nullptr;
PFNGLGETUNIFORMLOCATIONPROC                          glGetUniformLocation                          = nullptr;
PFNGLGETUNIFORMSUBROUTINEUIVPROC                     glGetUniformSubroutineuiv                     = nullptr;
PFNGLISPROGRAMPROC                                   glIsProgram                                   = nullptr;
PFNGLISSHADERPROC                                    glIsShader                                    = nullptr;
PFNGLLINKPROGRAMPROC                                 glLinkProgram                                 = nullptr;
PFNGLMINSAMPLESHADINGPROC                            glMinSampleShading                            = nullptr;
PFNGLPROGRAMBINARYPROC                               glProgramBinary                               = nullptr;
PFNGLPROGRAMPARAMETERIPROC                           glProgramParameteri                           = nullptr;
PFNGLPROGRAMUNIFORM1FPROC                            glProgramUniform1f                            = nullptr;
PFNGLPROGRAMUNIFORM1FVPROC                           glProgramUniform1fv                           = nullptr;
PFNGLPROGRAMUNIFORM1IPROC                            glProgramUniform1i                            = nullptr;
PFNGLPROGRAMUNIFORM1IVPROC                           glProgramUniform1iv                           = nullptr;
PFNGLPROGRAMUNIFORM1UIPROC                           glProgramUniform1ui                           = nullptr;
PFNGLPROGRAMUNIFORM1UIVPROC                          glProgramUniform1uiv                          = nullptr;
PFNGLPROGRAMUNIFORM2FPROC                            glProgramUniform2f                            = nullptr;
PFNGLPROGRAMUNIFORM2FVPROC                           glProgramUniform2fv                           = nullptr;
PFNGLPROGRAMUNIFORM2IPROC                            glProgramUniform2i                            = nullptr;
PFNGLPROGRAMUNIFORM2IVPROC                           glProgramUniform2iv                           = nullptr;
PFNGLPROGRAMUNIFORM2UIPROC                           glProgramUniform2ui                           = nullptr;
PFNGLPROGRAMUNIFORM2UIVPROC                          glProgramUniform2uiv                          = nullptr;
PFNGLPROGRAMUNIFORM3FPROC                            glProgramUniform3f                            = nullptr;
PFNGLPROGRAMUNIFORM3FVPROC                           glProgramUniform3fv                           = nullptr;
PFNGLPROGRAMUNIFORM3IPROC                            glProgramUniform3i                            = nullptr;
PFNGLPROGRAMUNIFORM3IVPROC                           glProgramUniform3iv                           = nullptr;
PFNGLPROGRAMUNIFORM3UIPROC                           glProgramUniform3ui                           = nullptr;
PFNGLPROGRAMUNIFORM3UIVPROC                          glProgramUniform3uiv                          = nullptr;
PFNGLPROGRAMUNIFORM4FPROC                            glProgramUniform4f                            = nullptr;
PFNGLPROGRAMUNIFORM4FVPROC                           glProgramUniform4fv                           = nullptr;
PFNGLPROGRAMUNIFORM4IPROC                            glProgramUniform4i                            = nullptr;
PFNGLPROGRAMUNIFORM4IVPROC                           glProgramUniform4iv                           = nullptr;
PFNGLPROGRAMUNIFORM4UIPROC                           glProgramUniform4ui                           = nullptr;
PFNGLPROGRAMUNIFORM4UIVPROC                          glProgramUniform4uiv                          = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC                     glProgramUniformMatrix2fv                     = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC                   glProgramUniformMatrix2x3fv                   = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC                   glProgramUniformMatrix2x4fv                   = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC                     glProgramUniformMatrix3fv                     = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC                   glProgramUniformMatrix3x2fv                   = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC                   glProgramUniformMatrix3x4fv                   = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC                     glProgramUniformMatrix4fv                     = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC                   glProgramUniformMatrix4x2fv                   = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC                   glProgramUniformMatrix4x3fv                   = nullptr;
PFNGLRELEASESHADERCOMPILERPROC                       glReleaseShaderCompiler                       = nullptr;
PFNGLSHADERBINARYPROC                                glShaderBinary                                = nullptr;
PFNGLSHADERSOURCEPROC                                glShaderSource                                = nullptr;
PFNGLUNIFORM1FPROC                                   glUniform1f                                   = nullptr;
PFNGLUNIFORM1FVPROC                                  glUniform1fv                                  = nullptr;
PFNGLUNIFORM1IPROC                                   glUniform1i                                   = nullptr;
PFNGLUNIFORM1IVPROC                                  glUniform1iv                                  = nullptr;
PFNGLUNIFORM1UIPROC                                  glUniform1ui                                  = nullptr;
PFNGLUNIFORM1UIVPROC                                 glUniform1uiv                                 = nullptr;
PFNGLUNIFORM2FPROC                                   glUniform2f                                   = nullptr;
PFNGLUNIFORM2FVPROC                                  glUniform2fv                                  = nullptr;
PFNGLUNIFORM2IPROC                                   glUniform2i                                   = nullptr;
PFNGLUNIFORM2IVPROC                                  glUniform2iv                                  = nullptr;
PFNGLUNIFORM2UIPROC                                  glUniform2ui                                  = nullptr;
PFNGLUNIFORM2UIVPROC                                 glUniform2uiv                                 = nullptr;
PFNGLUNIFORM3FPROC                                   glUniform3f                                   = nullptr;
PFNGLUNIFORM3FVPROC                                  glUniform3fv                                  = nullptr;
PFNGLUNIFORM3IPROC                                   glUniform3i                                   = nullptr;
PFNGLUNIFORM3IVPROC                                  glUniform3iv                                  = nullptr;
PFNGLUNIFORM3UIPROC                                  glUniform3ui                                  = nullptr;
PFNGLUNIFORM3UIVPROC                                 glUniform3uiv                                 = nullptr;
PFNGLUNIFORM4FPROC                                   glUniform4f                                   = nullptr;
PFNGLUNIFORM4FVPROC                                  glUniform4fv                                  = nullptr;
PFNGLUNIFORM4IPROC                                   glUniform4i                                   = nullptr;
PFNGLUNIFORM4IVPROC                                  glUniform4iv                                  = nullptr;
PFNGLUNIFORM4UIPROC                                  glUniform4ui                                  = nullptr;
PFNGLUNIFORM4UIVPROC                                 glUniform4uiv                                 = nullptr;
PFNGLUNIFORMMATRIX2FVPROC                            glUniformMatrix2fv                            = nullptr;
PFNGLUNIFORMMATRIX2X3FVPROC                          glUniformMatrix2x3fv                          = nullptr;
PFNGLUNIFORMMATRIX2X4FVPROC                          glUniformMatrix2x4fv                          = nullptr;
PFNGLUNIFORMMATRIX3FVPROC                            glUniformMatrix3fv                            = nullptr;
PFNGLUNIFORMMATRIX3X2FVPROC                          glUniformMatrix3x2fv                          = nullptr;
PFNGLUNIFORMMATRIX3X4FVPROC                          glUniformMatrix3x4fv                          = nullptr;
PFNGLUNIFORMMATRIX4FVPROC                            glUniformMatrix4fv                            = nullptr;
PFNGLUNIFORMMATRIX4X2FVPROC                          glUniformMatrix4x2fv                          = nullptr;
PFNGLUNIFORMMATRIX4X3FVPROC                          glUniformMatrix4x3fv                          = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC                         glUniformBlockBinding                         = nullptr;
PFNGLUNIFORMSUBROUTINESUIVPROC                       glUniformSubroutinesuiv                       = nullptr;
PFNGLUSEPROGRAMPROC                                  glUseProgram                                  = nullptr;
PFNGLUSEPROGRAMSTAGESPROC                            glUseProgramStages                            = nullptr;
PFNGLVALIDATEPROGRAMPROC                             glValidateProgram                             = nullptr;
PFNGLBINDBUFFERPROC                                  glBindBuffer                                  = nullptr;
PFNGLBINDBUFFERBASEPROC                              glBindBufferBase                              = nullptr;
PFNGLBINDBUFFERRANGEPROC                             glBindBufferRange                             = nullptr;
PFNGLBUFFERDATAPROC                                  glBufferData                                  = nullptr;
PFNGLBUFFERSUBDATAPROC                               glBufferSubData                               = nullptr;
PFNGLCOPYBUFFERSUBDATAPROC                           glCopyBufferSubData                           = nullptr;
PFNGLDELETEBUFFERSPROC                               glDeleteBuffers                               = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC                    glDisableVertexAttribArray                    = nullptr;
PFNGLDRAWARRAYSPROC                                  glDrawArrays                                  = nullptr;
PFNGLDRAWARRAYSINDIRECTPROC                          glDrawArraysIndirect                          = nullptr;
PFNGLDRAWARRAYSINSTANCEDPROC                         glDrawArraysInstanced                         = nullptr;
PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC             glDrawArraysInstancedBaseInstance             = nullptr;
PFNGLDRAWELEMENTSPROC                                glDrawElements                                = nullptr;
PFNGLDRAWELEMENTSBASEVERTEXPROC                      glDrawElementsBaseVertex                      = nullptr;
PFNGLDRAWELEMENTSINDIRECTPROC                        glDrawElementsIndirect                        = nullptr;
PFNGLDRAWELEMENTSINSTANCEDPROC                       glDrawElementsInstanced                       = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC           glDrawElementsInstancedBaseInstance           = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC             glDrawElementsInstancedBaseVertex             = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance = nullptr;
PFNGLDRAWRANGEELEMENTSPROC                           glDrawRangeElements                           = nullptr;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC                 glDrawRangeElementsBaseVertex                 = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC                     glEnableVertexAttribArray                     = nullptr;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC                      glFlushMappedBufferRange                      = nullptr;
PFNGLGENBUFFERSPROC                                  glGenBuffers                                  = nullptr;
PFNGLGETBUFFERPARAMETERI64VPROC                      glGetBufferParameteri64v                      = nullptr;
PFNGLGETBUFFERPARAMETERIVPROC                        glGetBufferParameteriv                        = nullptr;
PFNGLGETNAMEDBUFFERPARAMETERI64VPROC                 glGetNamedBufferParameteri64v                 = nullptr;
PFNGLGETNAMEDBUFFERPARAMETERIVPROC                   glGetNamedBufferParameteriv                   = nullptr;
PFNGLGETBUFFERPOINTERVPROC                           glGetBufferPointerv                           = nullptr;
PFNGLGETBUFFERSUBDATAPROC                            glGetBufferSubData                            = nullptr;
PFNGLGETVERTEXATTRIBPOINTERVPROC                     glGetVertexAttribPointerv                     = nullptr;
PFNGLISBUFFERPROC                                    glIsBuffer                                    = nullptr;
PFNGLMAPBUFFERPROC                                   glMapBuffer                                   = nullptr;
PFNGLMAPBUFFERRANGEPROC                              glMapBufferRange                              = nullptr;
PFNGLMULTIDRAWARRAYSPROC                             glMultiDrawArrays                             = nullptr;
PFNGLMULTIDRAWELEMENTSPROC                           glMultiDrawElements                           = nullptr;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC                 glMultiDrawElementsBaseVertex                 = nullptr;
PFNGLPRIMITIVERESTARTINDEXPROC                       glPrimitiveRestartIndex                       = nullptr;
PFNGLPROVOKINGVERTEXPROC                             glProvokingVertex                             = nullptr;
PFNGLUNMAPBUFFERPROC                                 glUnmapBuffer                                 = nullptr;
PFNGLVERTEXATTRIBDIVISORPROC                         glVertexAttribDivisor                         = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC                         glVertexAttribPointer                         = nullptr;
PFNGLBLENDCOLORPROC                                  glBlendColor                                  = nullptr;
PFNGLBLENDEQUATIONPROC                               glBlendEquation                               = nullptr;
PFNGLBLENDEQUATIONSEPARATEPROC                       glBlendEquationSeparate                       = nullptr;
PFNGLBLENDFUNCPROC                                   glBlendFunc                                   = nullptr;
PFNGLBLENDFUNCSEPARATEPROC                           glBlendFuncSeparate                           = nullptr;
PFNGLCLAMPCOLORPROC                                  glClampColor                                  = nullptr;
PFNGLCOLORMASKPROC                                   glColorMask                                   = nullptr;
PFNGLCULLFACEPROC                                    glCullFace                                    = nullptr;
PFNGLDEPTHFUNCPROC                                   glDepthFunc                                   = nullptr;
PFNGLDEPTHMASKPROC                                   glDepthMask                                   = nullptr;
PFNGLDEPTHRANGEPROC                                  glDepthRange                                  = nullptr;
PFNGLDEPTHRANGEINDEXEDPROC                           glDepthRangeIndexed                           = nullptr;
PFNGLDISABLEPROC                                     glDisable                                     = nullptr;
PFNGLENABLEPROC                                      glEnable                                      = nullptr;
PFNGLFRONTFACEPROC                                   glFrontFace                                   = nullptr;
PFNGLGETERRORPROC                                    glGetError                                    = nullptr;
PFNGLHINTPROC                                        glHint                                        = nullptr;
PFNGLISENABLEDPROC                                   glIsEnabled                                   = nullptr;
PFNGLLINEWIDTHPROC                                   glLineWidth                                   = nullptr;
PFNGLLOGICOPPROC                                     glLogicOp                                     = nullptr;
PFNGLPOINTSIZEPROC                                   glPointSize                                   = nullptr;
PFNGLPOLYGONMODEPROC                                 glPolygonMode                                 = nullptr;
PFNGLPOLYGONOFFSETPROC                               glPolygonOffset                               = nullptr;
PFNGLSAMPLECOVERAGEPROC                              glSampleCoverage                              = nullptr;
PFNGLSCISSORPROC                                     glScissor                                     = nullptr;
PFNGLSCISSORINDEXEDPROC                              glScissorIndexed                              = nullptr;
PFNGLSTENCILFUNCPROC                                 glStencilFunc                                 = nullptr;
PFNGLSTENCILFUNCSEPARATEPROC                         glStencilFuncSeparate                         = nullptr;
PFNGLSTENCILMASKPROC                                 glStencilMask                                 = nullptr;
PFNGLSTENCILMASKSEPARATEPROC                         glStencilMaskSeparate                         = nullptr;
PFNGLSTENCILOPPROC                                   glStencilOp                                   = nullptr;
PFNGLSTENCILOPSEPARATEPROC                           glStencilOpSeparate                           = nullptr;
PFNGLVIEWPORTPROC                                    glViewport                                    = nullptr;
PFNGLBEGINTRANSFORMFEEDBACKPROC                      glBeginTransformFeedback                      = nullptr;
PFNGLBINDTRANSFORMFEEDBACKPROC                       glBindTransformFeedback                       = nullptr;
PFNGLDELETETRANSFORMFEEDBACKSPROC                    glDeleteTransformFeedbacks                    = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKPROC                       glDrawTransformFeedback                       = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC              glDrawTransformFeedbackInstanced              = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC                 glDrawTransformFeedbackStream                 = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC        glDrawTransformFeedbackStreamInstanced        = nullptr;
PFNGLENDTRANSFORMFEEDBACKPROC                        glEndTransformFeedback                        = nullptr;
PFNGLGENTRANSFORMFEEDBACKSPROC                       glGenTransformFeedbacks                       = nullptr;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC                 glGetTransformFeedbackVarying                 = nullptr;
PFNGLISTRANSFORMFEEDBACKPROC                         glIsTransformFeedback                         = nullptr;
PFNGLPAUSETRANSFORMFEEDBACKPROC                      glPauseTransformFeedback                      = nullptr;
PFNGLRESUMETRANSFORMFEEDBACKPROC                     glResumeTransformFeedback                     = nullptr;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC                   glTransformFeedbackVaryings                   = nullptr;
PFNGLGETSTRINGPROC                                   glGetString                                   = nullptr;
PFNGLMEMORYBARRIERPROC                               glMemoryBarrier                               = nullptr;
PFNGLBEGINCONDITIONALRENDERPROC                      glBeginConditionalRender                      = nullptr;
PFNGLBEGINQUERYPROC                                  glBeginQuery                                  = nullptr;
PFNGLBEGINQUERYINDEXEDPROC                           glBeginQueryIndexed                           = nullptr;
PFNGLDELETEQUERIESPROC                               glDeleteQueries                               = nullptr;
PFNGLENDCONDITIONALRENDERPROC                        glEndConditionalRender                        = nullptr;
PFNGLENDQUERYPROC                                    glEndQuery                                    = nullptr;
PFNGLENDQUERYINDEXEDPROC                             glEndQueryIndexed                             = nullptr;
PFNGLGENQUERIESPROC                                  glGenQueries                                  = nullptr;
PFNGLGETQUERYIVPROC                                  glGetQueryiv                                  = nullptr;
PFNGLISQUERYPROC                                     glIsQuery                                     = nullptr;
PFNGLQUERYCOUNTERPROC                                glQueryCounter                                = nullptr;
PFNGLCLIENTWAITSYNCPROC                              glClientWaitSync                              = nullptr;
PFNGLDELETESYNCPROC                                  glDeleteSync                                  = nullptr;
PFNGLFENCESYNCPROC                                   glFenceSync                                   = nullptr;
PFNGLISSYNCPROC                                      glIsSync                                      = nullptr;
PFNGLWAITSYNCPROC                                    glWaitSync                                    = nullptr;
PFNGLBINDVERTEXARRAYPROC                             glBindVertexArray                             = nullptr;
PFNGLDELETEVERTEXARRAYSPROC                          glDeleteVertexArrays                          = nullptr;
PFNGLGENVERTEXARRAYSPROC                             glGenVertexArrays                             = nullptr;
PFNGLISVERTEXARRAYPROC                               glIsVertexArray                               = nullptr;
PFNGLBINDSAMPLERPROC                                 glBindSampler                                 = nullptr;
PFNGLDELETESAMPLERSPROC                              glDeleteSamplers                              = nullptr;
PFNGLGENSAMPLERSPROC                                 glGenSamplers                                 = nullptr;
PFNGLISSAMPLERPROC                                   glIsSampler                                   = nullptr;
PFNGLACTIVESHADERPROGRAMPROC                         glActiveShaderProgram                         = nullptr;
PFNGLBINDPROGRAMPIPELINEPROC                         glBindProgramPipeline                         = nullptr;
PFNGLDELETEPROGRAMPIPELINESPROC                      glDeleteProgramPipelines                      = nullptr;
PFNGLGENPROGRAMPIPELINESPROC                         glGenProgramPipelines                         = nullptr;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC                   glGetProgramPipelineInfoLog                   = nullptr;
PFNGLISPROGRAMPIPELINEPROC                           glIsProgramPipeline                           = nullptr;
PFNGLVALIDATEPROGRAMPIPELINEPROC                     glValidateProgramPipeline                     = nullptr;
PFNGLGETVERTEXATTRIBIIVPROC                          glGetVertexAttribIiv                          = nullptr;
PFNGLGETVERTEXATTRIBIUIVPROC                         glGetVertexAttribIuiv                         = nullptr;
PFNGLGETVERTEXATTRIBLDVPROC                          glGetVertexAttribLdv                          = nullptr;
PFNGLGETVERTEXATTRIBDVPROC                           glGetVertexAttribdv                           = nullptr;
PFNGLGETVERTEXATTRIBFVPROC                           glGetVertexAttribfv                           = nullptr;
PFNGLGETVERTEXATTRIBIVPROC                           glGetVertexAttribiv                           = nullptr;
PFNGLPATCHPARAMETERFVPROC                            glPatchParameterfv                            = nullptr;
PFNGLPATCHPARAMETERIPROC                             glPatchParameteri                             = nullptr;
PFNGLVERTEXATTRIB1DPROC                              glVertexAttrib1d                              = nullptr;
PFNGLVERTEXATTRIB1DVPROC                             glVertexAttrib1dv                             = nullptr;
PFNGLVERTEXATTRIB1FPROC                              glVertexAttrib1f                              = nullptr;
PFNGLVERTEXATTRIB1FVPROC                             glVertexAttrib1fv                             = nullptr;
PFNGLVERTEXATTRIB1SPROC                              glVertexAttrib1s                              = nullptr;
PFNGLVERTEXATTRIB1SVPROC                             glVertexAttrib1sv                             = nullptr;
PFNGLVERTEXATTRIB2DPROC                              glVertexAttrib2d                              = nullptr;
PFNGLVERTEXATTRIB2DVPROC                             glVertexAttrib2dv                             = nullptr;
PFNGLVERTEXATTRIB2FPROC                              glVertexAttrib2f                              = nullptr;
PFNGLVERTEXATTRIB2FVPROC                             glVertexAttrib2fv                             = nullptr;
PFNGLVERTEXATTRIB2SPROC                              glVertexAttrib2s                              = nullptr;
PFNGLVERTEXATTRIB2SVPROC                             glVertexAttrib2sv                             = nullptr;
PFNGLVERTEXATTRIB3DPROC                              glVertexAttrib3d                              = nullptr;
PFNGLVERTEXATTRIB3DVPROC                             glVertexAttrib3dv                             = nullptr;
PFNGLVERTEXATTRIB3FPROC                              glVertexAttrib3f                              = nullptr;
PFNGLVERTEXATTRIB3FVPROC                             glVertexAttrib3fv                             = nullptr;
PFNGLVERTEXATTRIB3SPROC                              glVertexAttrib3s                              = nullptr;
PFNGLVERTEXATTRIB3SVPROC                             glVertexAttrib3sv                             = nullptr;
PFNGLVERTEXATTRIB4NBVPROC                            glVertexAttrib4Nbv                            = nullptr;
PFNGLVERTEXATTRIB4NIVPROC                            glVertexAttrib4Niv                            = nullptr;
PFNGLVERTEXATTRIB4NSVPROC                            glVertexAttrib4Nsv                            = nullptr;
PFNGLVERTEXATTRIB4NUBPROC                            glVertexAttrib4Nub                            = nullptr;
PFNGLVERTEXATTRIB4NUBVPROC                           glVertexAttrib4Nubv                           = nullptr;
PFNGLVERTEXATTRIB4NUIVPROC                           glVertexAttrib4Nuiv                           = nullptr;
PFNGLVERTEXATTRIB4NUSVPROC                           glVertexAttrib4Nusv                           = nullptr;
PFNGLVERTEXATTRIB4BVPROC                             glVertexAttrib4bv                             = nullptr;
PFNGLVERTEXATTRIB4DPROC                              glVertexAttrib4d                              = nullptr;
PFNGLVERTEXATTRIB4DVPROC                             glVertexAttrib4dv                             = nullptr;
PFNGLVERTEXATTRIB4FPROC                              glVertexAttrib4f                              = nullptr;
PFNGLVERTEXATTRIB4FVPROC                             glVertexAttrib4fv                             = nullptr;
PFNGLVERTEXATTRIB4IVPROC                             glVertexAttrib4iv                             = nullptr;
PFNGLVERTEXATTRIB4SPROC                              glVertexAttrib4s                              = nullptr;
PFNGLVERTEXATTRIB4SVPROC                             glVertexAttrib4sv                             = nullptr;
PFNGLVERTEXATTRIB4UBVPROC                            glVertexAttrib4ubv                            = nullptr;
PFNGLVERTEXATTRIB4UIVPROC                            glVertexAttrib4uiv                            = nullptr;
PFNGLVERTEXATTRIB4USVPROC                            glVertexAttrib4usv                            = nullptr;
PFNGLVERTEXATTRIBI1IPROC                             glVertexAttribI1i                             = nullptr;
PFNGLVERTEXATTRIBI1IVPROC                            glVertexAttribI1iv                            = nullptr;
PFNGLVERTEXATTRIBI1UIPROC                            glVertexAttribI1ui                            = nullptr;
PFNGLVERTEXATTRIBI1UIVPROC                           glVertexAttribI1uiv                           = nullptr;
PFNGLVERTEXATTRIBI2IPROC                             glVertexAttribI2i                             = nullptr;
PFNGLVERTEXATTRIBI2IVPROC                            glVertexAttribI2iv                            = nullptr;
PFNGLVERTEXATTRIBI2UIPROC                            glVertexAttribI2ui                            = nullptr;
PFNGLVERTEXATTRIBI2UIVPROC                           glVertexAttribI2uiv                           = nullptr;
PFNGLVERTEXATTRIBI3IPROC                             glVertexAttribI3i                             = nullptr;
PFNGLVERTEXATTRIBI3IVPROC                            glVertexAttribI3iv                            = nullptr;
PFNGLVERTEXATTRIBI3UIPROC                            glVertexAttribI3ui                            = nullptr;
PFNGLVERTEXATTRIBI3UIVPROC                           glVertexAttribI3uiv                           = nullptr;
PFNGLVERTEXATTRIBI4BVPROC                            glVertexAttribI4bv                            = nullptr;
PFNGLVERTEXATTRIBI4IPROC                             glVertexAttribI4i                             = nullptr;
PFNGLVERTEXATTRIBI4IVPROC                            glVertexAttribI4iv                            = nullptr;
PFNGLVERTEXATTRIBI4SVPROC                            glVertexAttribI4sv                            = nullptr;
PFNGLVERTEXATTRIBI4UBVPROC                           glVertexAttribI4ubv                           = nullptr;
PFNGLVERTEXATTRIBI4UIPROC                            glVertexAttribI4ui                            = nullptr;
PFNGLVERTEXATTRIBI4UIVPROC                           glVertexAttribI4uiv                           = nullptr;
PFNGLVERTEXATTRIBI4USVPROC                           glVertexAttribI4usv                           = nullptr;
PFNGLVERTEXATTRIBL1DPROC                             glVertexAttribL1d                             = nullptr;
PFNGLVERTEXATTRIBL1DVPROC                            glVertexAttribL1dv                            = nullptr;
PFNGLVERTEXATTRIBL2DPROC                             glVertexAttribL2d                             = nullptr;
PFNGLVERTEXATTRIBL2DVPROC                            glVertexAttribL2dv                            = nullptr;
PFNGLVERTEXATTRIBL3DPROC                             glVertexAttribL3d                             = nullptr;
PFNGLVERTEXATTRIBL3DVPROC                            glVertexAttribL3dv                            = nullptr;
PFNGLVERTEXATTRIBL4DPROC                             glVertexAttribL4d                             = nullptr;
PFNGLVERTEXATTRIBL4DVPROC                            glVertexAttribL4dv                            = nullptr;
PFNGLVERTEXATTRIBP1UIPROC                            glVertexAttribP1ui                            = nullptr;
PFNGLVERTEXATTRIBP2UIPROC                            glVertexAttribP2ui                            = nullptr;
PFNGLVERTEXATTRIBP3UIPROC                            glVertexAttribP3ui                            = nullptr;
PFNGLVERTEXATTRIBP4UIPROC                            glVertexAttribP4ui                            = nullptr;
PFNGLDEPTHRANGEARRAYVPROC                            glDepthRangeArrayv                            = nullptr;
PFNGLGETBOOLEANI_VPROC                               glGetBooleani_v                               = nullptr;
PFNGLGETBOOLEANVPROC                                 glGetBooleanv                                 = nullptr;
PFNGLGETDOUBLEI_VPROC                                glGetDoublei_v                                = nullptr;
PFNGLGETDOUBLEVPROC                                  glGetDoublev                                  = nullptr;
PFNGLGETFLOATI_VPROC                                 glGetFloati_v                                 = nullptr;
PFNGLGETFLOATVPROC                                   glGetFloatv                                   = nullptr;
PFNGLGETINTEGER64I_VPROC                             glGetInteger64i_v                             = nullptr;
PFNGLGETINTEGER64VPROC                               glGetInteger64v                               = nullptr;
PFNGLGETINTEGERI_VPROC                               glGetIntegeri_v                               = nullptr;
PFNGLGETINTEGERVPROC                                 glGetIntegerv                                 = nullptr;
PFNGLPIXELSTOREFPROC                                 glPixelStoref                                 = nullptr;
PFNGLPIXELSTOREIPROC                                 glPixelStorei                                 = nullptr;
PFNGLPOINTPARAMETERFPROC                             glPointParameterf                             = nullptr;
PFNGLPOINTPARAMETERFVPROC                            glPointParameterfv                            = nullptr;
PFNGLPOINTPARAMETERIPROC                             glPointParameteri                             = nullptr;
PFNGLPOINTPARAMETERIVPROC                            glPointParameteriv                            = nullptr;
PFNGLSCISSORARRAYVPROC                               glScissorArrayv                               = nullptr;
PFNGLVIEWPORTARRAYVPROC                              glViewportArrayv                              = nullptr;
PFNGLGETINTERNALFORMATI64VPROC                       glGetInternalformati64v                       = nullptr;
PFNGLGETINTERNALFORMATIVPROC                         glGetInternalformativ                         = nullptr;
PFNGLVIEWPORTINDEXEDFPROC                            glViewportIndexedf                            = nullptr;
PFNGLVIEWPORTINDEXEDFVPROC                           glViewportIndexedfv                           = nullptr;
PFNGLGETMULTISAMPLEFVPROC                            glGetMultisamplefv                            = nullptr;
PFNGLGETQUERYINDEXEDIVPROC                           glGetQueryIndexediv                           = nullptr;
PFNGLGETQUERYOBJECTI64VPROC                          glGetQueryObjecti64v                          = nullptr;
PFNGLGETQUERYOBJECTIVPROC                            glGetQueryObjectiv                            = nullptr;
PFNGLGETQUERYOBJECTUI64VPROC                         glGetQueryObjectui64v                         = nullptr;
PFNGLGETQUERYOBJECTUIVPROC                           glGetQueryObjectuiv                           = nullptr;
PFNGLGETSYNCIVPROC                                   glGetSynciv                                   = nullptr;
PFNGLGETSAMPLERPARAMETERIIVPROC                      glGetSamplerParameterIiv                      = nullptr;
PFNGLGETSAMPLERPARAMETERIUIVPROC                     glGetSamplerParameterIuiv                     = nullptr;
PFNGLGETSAMPLERPARAMETERFVPROC                       glGetSamplerParameterfv                       = nullptr;
PFNGLGETSAMPLERPARAMETERIVPROC                       glGetSamplerParameteriv                       = nullptr;
PFNGLSAMPLERPARAMETERIIVPROC                         glSamplerParameterIiv                         = nullptr;
PFNGLSAMPLERPARAMETERIUIVPROC                        glSamplerParameterIuiv                        = nullptr;
PFNGLSAMPLERPARAMETERFPROC                           glSamplerParameterf                           = nullptr;
PFNGLSAMPLERPARAMETERFVPROC                          glSamplerParameterfv                          = nullptr;
PFNGLSAMPLERPARAMETERIPROC                           glSamplerParameteri                           = nullptr;
PFNGLSAMPLERPARAMETERIVPROC                          glSamplerParameteriv                          = nullptr;
PFNGLGETPROGRAMPIPELINEIVPROC                        glGetProgramPipelineiv                        = nullptr;

void bindNewWGLFunctions() {
  GL_BIND_FUNCTION(wglGetExtensionsStringARB);
  GL_BIND_FUNCTION(wglChoosePixelFormatARB);
  GL_BIND_FUNCTION(wglCreateContextAttribsARB);
}

void bindGLFunctions() {
  wglGetTypedProcAddress( &glAttachShader, "glAttachShader" );
  GL_BIND_FUNCTION(glActiveTexture);
  GL_BIND_FUNCTION(glBindImageTexture);
  GL_BIND_FUNCTION(glBindTexture);
  GL_BIND_FUNCTION(glCompressedTexImage1D);
  GL_BIND_FUNCTION(glCompressedTexImage2D);
  GL_BIND_FUNCTION(glCompressedTexImage3D);
  GL_BIND_FUNCTION(glCompressedTexSubImage1D);
  GL_BIND_FUNCTION(glCompressedTexSubImage2D);
  GL_BIND_FUNCTION(glCompressedTexSubImage3D);
  GL_BIND_FUNCTION(glCopyTexImage1D);
  GL_BIND_FUNCTION(glCopyTexImage2D);
  GL_BIND_FUNCTION(glCopyTexSubImage1D);
  GL_BIND_FUNCTION(glCopyTexSubImage2D);
  GL_BIND_FUNCTION(glCopyTexSubImage3D);
  GL_BIND_FUNCTION(glDeleteTextures);
  GL_BIND_FUNCTION(glGenTextures);
  GL_BIND_FUNCTION(glGetCompressedTexImage);
  GL_BIND_FUNCTION(glGetTexImage);
  GL_BIND_FUNCTION(glGetTexLevelParameterfv);
  GL_BIND_FUNCTION(glGetTexLevelParameteriv);
  GL_BIND_FUNCTION(glGetTexParameterfv);
  GL_BIND_FUNCTION(glGetTexParameteriv);
  GL_BIND_FUNCTION(glIsTexture);
  GL_BIND_FUNCTION(glTexBuffer);
  GL_BIND_FUNCTION(glTexImage1D);
  GL_BIND_FUNCTION(glTexImage2D);
  GL_BIND_FUNCTION(glTexImage2DMultisample);
  GL_BIND_FUNCTION(glTexImage3D);
  GL_BIND_FUNCTION(glTexImage3DMultisample);
  GL_BIND_FUNCTION(glTexParameterIiv);
  GL_BIND_FUNCTION(glTexParameterIuiv);
  GL_BIND_FUNCTION(glTexParameterf);
  GL_BIND_FUNCTION(glTexParameterfv);
  GL_BIND_FUNCTION(glTexParameteri);
  GL_BIND_FUNCTION(glTexParameteriv);
  GL_BIND_FUNCTION(glTexStorage1D);
  GL_BIND_FUNCTION(glTexStorage2D);
  GL_BIND_FUNCTION(glTexStorage3D);
  GL_BIND_FUNCTION(glTexSubImage1D);
  GL_BIND_FUNCTION(glTexSubImage2D);
  GL_BIND_FUNCTION(glTexSubImage3D);
  GL_BIND_FUNCTION(glClear);
  GL_BIND_FUNCTION(glClearBufferfi);
  GL_BIND_FUNCTION(glClearBufferfv);
  GL_BIND_FUNCTION(glClearBufferiv);
  GL_BIND_FUNCTION(glClearBufferuiv);
  GL_BIND_FUNCTION(glClearColor);
  GL_BIND_FUNCTION(glClearDepth);
  GL_BIND_FUNCTION(glClearDepthf);
  GL_BIND_FUNCTION(glClearStencil);
  GL_BIND_FUNCTION(glDrawBuffer);
  GL_BIND_FUNCTION(glFinish);
  GL_BIND_FUNCTION(glFlush);
  GL_BIND_FUNCTION(glReadBuffer);
  GL_BIND_FUNCTION(glReadPixels);
  GL_BIND_FUNCTION(glBindFramebuffer);
  GL_BIND_FUNCTION(glBindRenderbuffer);
  GL_BIND_FUNCTION(glBlitFramebuffer);
  GL_BIND_FUNCTION(glCheckFramebufferStatus);
  GL_BIND_FUNCTION(glDeleteFramebuffers);
  GL_BIND_FUNCTION(glDeleteRenderbuffers);
  GL_BIND_FUNCTION(glDrawBuffers);
  GL_BIND_FUNCTION(glFramebufferRenderbuffer);
  GL_BIND_FUNCTION(glFramebufferTexture);
  GL_BIND_FUNCTION(glFramebufferTextureLayer);
  GL_BIND_FUNCTION(glGenFramebuffers);
  GL_BIND_FUNCTION(glGenRenderbuffers);
  GL_BIND_FUNCTION(glGenerateMipmap);
  GL_BIND_FUNCTION(glGetFramebufferAttachmentParameteriv);
  GL_BIND_FUNCTION(glGetNamedFramebufferAttachmentParameteriv);
  GL_BIND_FUNCTION(glGetNamedRenderbufferParameteriv);
  GL_BIND_FUNCTION(glGetRenderbufferParameteriv);
  GL_BIND_FUNCTION(glIsFramebuffer);
  GL_BIND_FUNCTION(glIsRenderbuffer);
  GL_BIND_FUNCTION(glRenderbufferStorage);
  GL_BIND_FUNCTION(glRenderbufferStorageMultisample);
  GL_BIND_FUNCTION(glSampleMaski);
  GL_BIND_FUNCTION(glAttachShader);
  GL_BIND_FUNCTION(glBindAttribLocation);
  GL_BIND_FUNCTION(glBindFragDataLocation);
  GL_BIND_FUNCTION(glBindFragDataLocationIndexed);
  GL_BIND_FUNCTION(glCompileShader);
  GL_BIND_FUNCTION(glCreateProgram);
  GL_BIND_FUNCTION(glCreateShader);
  GL_BIND_FUNCTION(glCreateShaderProgramv);
  GL_BIND_FUNCTION(glDeleteProgram);
  GL_BIND_FUNCTION(glDeleteShader);
  GL_BIND_FUNCTION(glDetachShader);
  GL_BIND_FUNCTION(glGetActiveAtomicCounterBufferiv);
  GL_BIND_FUNCTION(glGetActiveAttrib);
  GL_BIND_FUNCTION(glGetActiveSubroutineName);
  GL_BIND_FUNCTION(glGetActiveSubroutineUniformiv);
  GL_BIND_FUNCTION(glGetActiveSubroutineUniformName);
  GL_BIND_FUNCTION(glGetActiveUniform);
  GL_BIND_FUNCTION(glGetActiveUniformBlockiv);
  GL_BIND_FUNCTION(glGetActiveUniformBlockName);
  GL_BIND_FUNCTION(glGetActiveUniformName);
  GL_BIND_FUNCTION(glGetActiveUniformsiv);
  GL_BIND_FUNCTION(glGetAttachedShaders);
  GL_BIND_FUNCTION(glGetAttribLocation);
  GL_BIND_FUNCTION(glGetFragDataIndex);
  GL_BIND_FUNCTION(glGetFragDataLocation);
  GL_BIND_FUNCTION(glGetProgramiv);
  GL_BIND_FUNCTION(glGetProgramBinary);
  GL_BIND_FUNCTION(glGetProgramInfoLog);
  GL_BIND_FUNCTION(glGetProgramStageiv);
  GL_BIND_FUNCTION(glGetShaderiv);
  GL_BIND_FUNCTION(glGetShaderInfoLog);
  GL_BIND_FUNCTION(glGetShaderPrecisionFormat);
  GL_BIND_FUNCTION(glGetShaderSource);
  GL_BIND_FUNCTION(glGetSubroutineIndex);
  GL_BIND_FUNCTION(glGetSubroutineUniformLocation);
  GL_BIND_FUNCTION(glGetUniformdv);
  GL_BIND_FUNCTION(glGetUniformfv);
  GL_BIND_FUNCTION(glGetUniformiv);
  GL_BIND_FUNCTION(glGetUniformuiv);
  GL_BIND_FUNCTION(glGetnUniformdv);
  GL_BIND_FUNCTION(glGetnUniformfv);
  GL_BIND_FUNCTION(glGetnUniformiv);
  GL_BIND_FUNCTION(glGetnUniformuiv);
  GL_BIND_FUNCTION(glGetUniformBlockIndex);
  GL_BIND_FUNCTION(glGetUniformIndices);
  GL_BIND_FUNCTION(glGetUniformLocation);
  GL_BIND_FUNCTION(glGetUniformSubroutineuiv);
  GL_BIND_FUNCTION(glIsProgram);
  GL_BIND_FUNCTION(glIsShader);
  GL_BIND_FUNCTION(glLinkProgram);
  GL_BIND_FUNCTION(glMinSampleShading);
  GL_BIND_FUNCTION(glProgramBinary);
  GL_BIND_FUNCTION(glProgramParameteri);
  GL_BIND_FUNCTION(glProgramUniform1f);
  GL_BIND_FUNCTION(glProgramUniform1fv);
  GL_BIND_FUNCTION(glProgramUniform1i);
  GL_BIND_FUNCTION(glProgramUniform1iv);
  GL_BIND_FUNCTION(glProgramUniform1ui);
  GL_BIND_FUNCTION(glProgramUniform1uiv);
  GL_BIND_FUNCTION(glProgramUniform2f);
  GL_BIND_FUNCTION(glProgramUniform2fv);
  GL_BIND_FUNCTION(glProgramUniform2i);
  GL_BIND_FUNCTION(glProgramUniform2iv);
  GL_BIND_FUNCTION(glProgramUniform2ui);
  GL_BIND_FUNCTION(glProgramUniform2uiv);
  GL_BIND_FUNCTION(glProgramUniform3f);
  GL_BIND_FUNCTION(glProgramUniform3fv);
  GL_BIND_FUNCTION(glProgramUniform3i);
  GL_BIND_FUNCTION(glProgramUniform3iv);
  GL_BIND_FUNCTION(glProgramUniform3ui);
  GL_BIND_FUNCTION(glProgramUniform3uiv);
  GL_BIND_FUNCTION(glProgramUniform4f);
  GL_BIND_FUNCTION(glProgramUniform4fv);
  GL_BIND_FUNCTION(glProgramUniform4i);
  GL_BIND_FUNCTION(glProgramUniform4iv);
  GL_BIND_FUNCTION(glProgramUniform4ui);
  GL_BIND_FUNCTION(glProgramUniform4uiv);
  GL_BIND_FUNCTION(glProgramUniformMatrix2fv);
  GL_BIND_FUNCTION(glProgramUniformMatrix2x3fv);
  GL_BIND_FUNCTION(glProgramUniformMatrix2x4fv);
  GL_BIND_FUNCTION(glProgramUniformMatrix3fv);
  GL_BIND_FUNCTION(glProgramUniformMatrix3x2fv);
  GL_BIND_FUNCTION(glProgramUniformMatrix3x4fv);
  GL_BIND_FUNCTION(glProgramUniformMatrix4fv);
  GL_BIND_FUNCTION(glProgramUniformMatrix4x2fv);
  GL_BIND_FUNCTION(glProgramUniformMatrix4x3fv);
  GL_BIND_FUNCTION(glReleaseShaderCompiler);
  GL_BIND_FUNCTION(glShaderBinary);
  GL_BIND_FUNCTION(glShaderSource);
  GL_BIND_FUNCTION(glUniform1f);
  GL_BIND_FUNCTION(glUniform1fv);
  GL_BIND_FUNCTION(glUniform1i);
  GL_BIND_FUNCTION(glUniform1iv);
  GL_BIND_FUNCTION(glUniform1ui);
  GL_BIND_FUNCTION(glUniform1uiv);
  GL_BIND_FUNCTION(glUniform2f);
  GL_BIND_FUNCTION(glUniform2fv);
  GL_BIND_FUNCTION(glUniform2i);
  GL_BIND_FUNCTION(glUniform2iv);
  GL_BIND_FUNCTION(glUniform2ui);
  GL_BIND_FUNCTION(glUniform2uiv);
  GL_BIND_FUNCTION(glUniform3f);
  GL_BIND_FUNCTION(glUniform3fv);
  GL_BIND_FUNCTION(glUniform3i);
  GL_BIND_FUNCTION(glUniform3iv);
  GL_BIND_FUNCTION(glUniform3ui);
  GL_BIND_FUNCTION(glUniform3uiv);
  GL_BIND_FUNCTION(glUniform4f);
  GL_BIND_FUNCTION(glUniform4fv);
  GL_BIND_FUNCTION(glUniform4i);
  GL_BIND_FUNCTION(glUniform4iv);
  GL_BIND_FUNCTION(glUniform4ui);
  GL_BIND_FUNCTION(glUniform4uiv);
  GL_BIND_FUNCTION(glUniformMatrix2fv);
  GL_BIND_FUNCTION(glUniformMatrix2x3fv);
  GL_BIND_FUNCTION(glUniformMatrix2x4fv);
  GL_BIND_FUNCTION(glUniformMatrix3fv);
  GL_BIND_FUNCTION(glUniformMatrix3x2fv);
  GL_BIND_FUNCTION(glUniformMatrix3x4fv);
  GL_BIND_FUNCTION(glUniformMatrix4fv);
  GL_BIND_FUNCTION(glUniformMatrix4x2fv);
  GL_BIND_FUNCTION(glUniformMatrix4x3fv);
  GL_BIND_FUNCTION(glUniformBlockBinding);
  GL_BIND_FUNCTION(glUniformSubroutinesuiv);
  GL_BIND_FUNCTION(glUseProgram);
  GL_BIND_FUNCTION(glUseProgramStages);
  GL_BIND_FUNCTION(glValidateProgram);
  GL_BIND_FUNCTION(glBindBuffer);
  GL_BIND_FUNCTION(glBindBufferBase);
  GL_BIND_FUNCTION(glBindBufferRange);
  GL_BIND_FUNCTION(glBufferData);
  GL_BIND_FUNCTION(glBufferSubData);
  GL_BIND_FUNCTION(glCopyBufferSubData);
  GL_BIND_FUNCTION(glDeleteBuffers);
  GL_BIND_FUNCTION(glDisableVertexAttribArray);
  GL_BIND_FUNCTION(glDrawArrays);
  GL_BIND_FUNCTION(glDrawArraysIndirect);
  GL_BIND_FUNCTION(glDrawArraysInstanced);
  GL_BIND_FUNCTION(glDrawArraysInstancedBaseInstance);
  GL_BIND_FUNCTION(glDrawElements);
  GL_BIND_FUNCTION(glDrawElementsBaseVertex);
  GL_BIND_FUNCTION(glDrawElementsIndirect);
  GL_BIND_FUNCTION(glDrawElementsInstanced);
  GL_BIND_FUNCTION(glDrawElementsInstancedBaseInstance);
  GL_BIND_FUNCTION(glDrawElementsInstancedBaseVertex);
  GL_BIND_FUNCTION(glDrawElementsInstancedBaseVertexBaseInstance);
  GL_BIND_FUNCTION(glDrawRangeElements);
  GL_BIND_FUNCTION(glDrawRangeElementsBaseVertex);
  GL_BIND_FUNCTION(glEnableVertexAttribArray);
  GL_BIND_FUNCTION(glFlushMappedBufferRange);
  GL_BIND_FUNCTION(glGenBuffers);
  GL_BIND_FUNCTION(glGetBufferParameteri64v);
  GL_BIND_FUNCTION(glGetBufferParameteriv);
  GL_BIND_FUNCTION(glGetNamedBufferParameteri64v);
  GL_BIND_FUNCTION(glGetNamedBufferParameteriv);
  GL_BIND_FUNCTION(glGetBufferPointerv);
  GL_BIND_FUNCTION(glGetBufferSubData);
  GL_BIND_FUNCTION(glGetVertexAttribPointerv);
  GL_BIND_FUNCTION(glIsBuffer);
  GL_BIND_FUNCTION(glMapBuffer);
  GL_BIND_FUNCTION(glMapBufferRange);
  GL_BIND_FUNCTION(glMultiDrawArrays);
  GL_BIND_FUNCTION(glMultiDrawElements);
  GL_BIND_FUNCTION(glMultiDrawElementsBaseVertex);
  GL_BIND_FUNCTION(glPrimitiveRestartIndex);
  GL_BIND_FUNCTION(glProvokingVertex);
  GL_BIND_FUNCTION(glUnmapBuffer);
  GL_BIND_FUNCTION(glVertexAttribDivisor);
  GL_BIND_FUNCTION(glVertexAttribPointer);
  GL_BIND_FUNCTION(glBlendColor);
  GL_BIND_FUNCTION(glBlendEquation);
  GL_BIND_FUNCTION(glBlendEquationSeparate);
  GL_BIND_FUNCTION(glBlendFunc);
  GL_BIND_FUNCTION(glBlendFuncSeparate);
  GL_BIND_FUNCTION(glClampColor);
  GL_BIND_FUNCTION(glColorMask);
  GL_BIND_FUNCTION(glCullFace);
  GL_BIND_FUNCTION(glDepthFunc);
  GL_BIND_FUNCTION(glDepthMask);
  GL_BIND_FUNCTION(glDepthRange);
  GL_BIND_FUNCTION(glDepthRangeIndexed);
  GL_BIND_FUNCTION(glDisable);
  GL_BIND_FUNCTION(glEnable);
  GL_BIND_FUNCTION(glFrontFace);
  GL_BIND_FUNCTION(glGetError);
  GL_BIND_FUNCTION(glHint);
  GL_BIND_FUNCTION(glIsEnabled);
  GL_BIND_FUNCTION(glLineWidth);
  GL_BIND_FUNCTION(glLogicOp);
  GL_BIND_FUNCTION(glPointSize);
  GL_BIND_FUNCTION(glPolygonMode);
  GL_BIND_FUNCTION(glPolygonOffset);
  GL_BIND_FUNCTION(glSampleCoverage);
  GL_BIND_FUNCTION(glScissor);
  GL_BIND_FUNCTION(glScissorIndexed);
  GL_BIND_FUNCTION(glStencilFunc);
  GL_BIND_FUNCTION(glStencilFuncSeparate);
  GL_BIND_FUNCTION(glStencilMask);
  GL_BIND_FUNCTION(glStencilMaskSeparate);
  GL_BIND_FUNCTION(glStencilOp);
  GL_BIND_FUNCTION(glStencilOpSeparate);
  GL_BIND_FUNCTION(glViewport);
  GL_BIND_FUNCTION(glBeginTransformFeedback);
  GL_BIND_FUNCTION(glBindTransformFeedback);
  GL_BIND_FUNCTION(glDeleteTransformFeedbacks);
  GL_BIND_FUNCTION(glDrawTransformFeedback);
  GL_BIND_FUNCTION(glDrawTransformFeedbackInstanced);
  GL_BIND_FUNCTION(glDrawTransformFeedbackStream);
  GL_BIND_FUNCTION(glDrawTransformFeedbackStreamInstanced);
  GL_BIND_FUNCTION(glEndTransformFeedback);
  GL_BIND_FUNCTION(glGenTransformFeedbacks);
  GL_BIND_FUNCTION(glGetTransformFeedbackVarying);
  GL_BIND_FUNCTION(glIsTransformFeedback);
  GL_BIND_FUNCTION(glPauseTransformFeedback);
  GL_BIND_FUNCTION(glResumeTransformFeedback);
  GL_BIND_FUNCTION(glTransformFeedbackVaryings);
  GL_BIND_FUNCTION(glGetString);
  GL_BIND_FUNCTION(glMemoryBarrier);
  GL_BIND_FUNCTION(glBeginConditionalRender);
  GL_BIND_FUNCTION(glBeginQuery);
  GL_BIND_FUNCTION(glBeginQueryIndexed);
  GL_BIND_FUNCTION(glDeleteQueries);
  GL_BIND_FUNCTION(glEndConditionalRender);
  GL_BIND_FUNCTION(glEndQuery);
  GL_BIND_FUNCTION(glEndQueryIndexed);
  GL_BIND_FUNCTION(glGenQueries);
  GL_BIND_FUNCTION(glGetQueryiv);
  GL_BIND_FUNCTION(glIsQuery);
  GL_BIND_FUNCTION(glQueryCounter);
  GL_BIND_FUNCTION(glClientWaitSync);
  GL_BIND_FUNCTION(glDeleteSync);
  GL_BIND_FUNCTION(glFenceSync);
  GL_BIND_FUNCTION(glIsSync);
  GL_BIND_FUNCTION(glWaitSync);
  GL_BIND_FUNCTION(glBindVertexArray);
  GL_BIND_FUNCTION(glDeleteVertexArrays);
  GL_BIND_FUNCTION(glGenVertexArrays);
  GL_BIND_FUNCTION(glIsVertexArray);
  GL_BIND_FUNCTION(glBindSampler);
  GL_BIND_FUNCTION(glDeleteSamplers);
  GL_BIND_FUNCTION(glGenSamplers);
  GL_BIND_FUNCTION(glIsSampler);
  GL_BIND_FUNCTION(glActiveShaderProgram);
  GL_BIND_FUNCTION(glBindProgramPipeline);
  GL_BIND_FUNCTION(glDeleteProgramPipelines);
  GL_BIND_FUNCTION(glGenProgramPipelines);
  GL_BIND_FUNCTION(glGetProgramPipelineInfoLog);
  GL_BIND_FUNCTION(glIsProgramPipeline);
  GL_BIND_FUNCTION(glValidateProgramPipeline);
  GL_BIND_FUNCTION(glGetVertexAttribIiv);
  GL_BIND_FUNCTION(glGetVertexAttribIuiv);
  GL_BIND_FUNCTION(glGetVertexAttribLdv);
  GL_BIND_FUNCTION(glGetVertexAttribdv);
  GL_BIND_FUNCTION(glGetVertexAttribfv);
  GL_BIND_FUNCTION(glGetVertexAttribiv);
  GL_BIND_FUNCTION(glPatchParameterfv);
  GL_BIND_FUNCTION(glPatchParameteri);
  GL_BIND_FUNCTION(glVertexAttrib1d);
  GL_BIND_FUNCTION(glVertexAttrib1dv);
  GL_BIND_FUNCTION(glVertexAttrib1f);
  GL_BIND_FUNCTION(glVertexAttrib1fv);
  GL_BIND_FUNCTION(glVertexAttrib1s);
  GL_BIND_FUNCTION(glVertexAttrib1sv);
  GL_BIND_FUNCTION(glVertexAttrib2d);
  GL_BIND_FUNCTION(glVertexAttrib2dv);
  GL_BIND_FUNCTION(glVertexAttrib2f);
  GL_BIND_FUNCTION(glVertexAttrib2fv);
  GL_BIND_FUNCTION(glVertexAttrib2s);
  GL_BIND_FUNCTION(glVertexAttrib2sv);
  GL_BIND_FUNCTION(glVertexAttrib3d);
  GL_BIND_FUNCTION(glVertexAttrib3dv);
  GL_BIND_FUNCTION(glVertexAttrib3f);
  GL_BIND_FUNCTION(glVertexAttrib3fv);
  GL_BIND_FUNCTION(glVertexAttrib3s);
  GL_BIND_FUNCTION(glVertexAttrib3sv);
  GL_BIND_FUNCTION(glVertexAttrib4Nbv);
  GL_BIND_FUNCTION(glVertexAttrib4Niv);
  GL_BIND_FUNCTION(glVertexAttrib4Nsv);
  GL_BIND_FUNCTION(glVertexAttrib4Nub);
  GL_BIND_FUNCTION(glVertexAttrib4Nubv);
  GL_BIND_FUNCTION(glVertexAttrib4Nuiv);
  GL_BIND_FUNCTION(glVertexAttrib4Nusv);
  GL_BIND_FUNCTION(glVertexAttrib4bv);
  GL_BIND_FUNCTION(glVertexAttrib4d);
  GL_BIND_FUNCTION(glVertexAttrib4dv);
  GL_BIND_FUNCTION(glVertexAttrib4f);
  GL_BIND_FUNCTION(glVertexAttrib4fv);
  GL_BIND_FUNCTION(glVertexAttrib4iv);
  GL_BIND_FUNCTION(glVertexAttrib4s);
  GL_BIND_FUNCTION(glVertexAttrib4sv);
  GL_BIND_FUNCTION(glVertexAttrib4ubv);
  GL_BIND_FUNCTION(glVertexAttrib4uiv);
  GL_BIND_FUNCTION(glVertexAttrib4usv);
  GL_BIND_FUNCTION(glVertexAttribI1i);
  GL_BIND_FUNCTION(glVertexAttribI1iv);
  GL_BIND_FUNCTION(glVertexAttribI1ui);
  GL_BIND_FUNCTION(glVertexAttribI1uiv);
  GL_BIND_FUNCTION(glVertexAttribI2i);
  GL_BIND_FUNCTION(glVertexAttribI2iv);
  GL_BIND_FUNCTION(glVertexAttribI2ui);
  GL_BIND_FUNCTION(glVertexAttribI2uiv);
  GL_BIND_FUNCTION(glVertexAttribI3i);
  GL_BIND_FUNCTION(glVertexAttribI3iv);
  GL_BIND_FUNCTION(glVertexAttribI3ui);
  GL_BIND_FUNCTION(glVertexAttribI3uiv);
  GL_BIND_FUNCTION(glVertexAttribI4bv);
  GL_BIND_FUNCTION(glVertexAttribI4i);
  GL_BIND_FUNCTION(glVertexAttribI4iv);
  GL_BIND_FUNCTION(glVertexAttribI4sv);
  GL_BIND_FUNCTION(glVertexAttribI4ubv);
  GL_BIND_FUNCTION(glVertexAttribI4ui);
  GL_BIND_FUNCTION(glVertexAttribI4uiv);
  GL_BIND_FUNCTION(glVertexAttribI4usv);
  GL_BIND_FUNCTION(glVertexAttribL1d);
  GL_BIND_FUNCTION(glVertexAttribL1dv);
  GL_BIND_FUNCTION(glVertexAttribL2d);
  GL_BIND_FUNCTION(glVertexAttribL2dv);
  GL_BIND_FUNCTION(glVertexAttribL3d);
  GL_BIND_FUNCTION(glVertexAttribL3dv);
  GL_BIND_FUNCTION(glVertexAttribL4d);
  GL_BIND_FUNCTION(glVertexAttribL4dv);
  GL_BIND_FUNCTION(glVertexAttribP1ui);
  GL_BIND_FUNCTION(glVertexAttribP2ui);
  GL_BIND_FUNCTION(glVertexAttribP3ui);
  GL_BIND_FUNCTION(glVertexAttribP4ui);
  GL_BIND_FUNCTION(glDepthRangeArrayv);
  GL_BIND_FUNCTION(glGetBooleani_v);
  GL_BIND_FUNCTION(glGetBooleanv);
  GL_BIND_FUNCTION(glGetDoublei_v);
  GL_BIND_FUNCTION(glGetDoublev);
  GL_BIND_FUNCTION(glGetFloati_v);
  GL_BIND_FUNCTION(glGetFloatv);
  GL_BIND_FUNCTION(glGetInteger64i_v);
  GL_BIND_FUNCTION(glGetInteger64v);
  GL_BIND_FUNCTION(glGetIntegeri_v);
  GL_BIND_FUNCTION(glGetIntegerv);
  GL_BIND_FUNCTION(glPixelStoref);
  GL_BIND_FUNCTION(glPixelStorei);
  GL_BIND_FUNCTION(glPointParameterf);
  GL_BIND_FUNCTION(glPointParameterfv);
  GL_BIND_FUNCTION(glPointParameteri);
  GL_BIND_FUNCTION(glPointParameteriv);
  GL_BIND_FUNCTION(glScissorArrayv);
  GL_BIND_FUNCTION(glViewportArrayv);
  GL_BIND_FUNCTION(glGetInternalformati64v);
  GL_BIND_FUNCTION(glGetInternalformativ);
  GL_BIND_FUNCTION(glViewportIndexedf);
  GL_BIND_FUNCTION(glViewportIndexedfv);
  GL_BIND_FUNCTION(glGetMultisamplefv);
  GL_BIND_FUNCTION(glGetQueryIndexediv);
  GL_BIND_FUNCTION(glGetQueryObjecti64v);
  GL_BIND_FUNCTION(glGetQueryObjectiv);
  GL_BIND_FUNCTION(glGetQueryObjectui64v);
  GL_BIND_FUNCTION(glGetQueryObjectuiv);
  GL_BIND_FUNCTION(glGetSynciv);
  GL_BIND_FUNCTION(glGetSamplerParameterIiv);
  GL_BIND_FUNCTION(glGetSamplerParameterIuiv);
  GL_BIND_FUNCTION(glGetSamplerParameterfv);
  GL_BIND_FUNCTION(glGetSamplerParameteriv);
  GL_BIND_FUNCTION(glSamplerParameterIiv);
  GL_BIND_FUNCTION(glSamplerParameterIuiv);
  GL_BIND_FUNCTION(glSamplerParameterf);
  GL_BIND_FUNCTION(glSamplerParameterfv);
  GL_BIND_FUNCTION(glSamplerParameteri);
  GL_BIND_FUNCTION(glSamplerParameteriv);
  GL_BIND_FUNCTION(glGetProgramPipelineiv);
}

size_t GLSize(GLint type) {
  switch(type) {
    default:
      ERROR_AND_DIE("unsupport type");
  }
}

//enum eDataDeclType {
//  MP_FLOAT = 0,
//  MP_BYTE,
//  MP_NUM_DATA_DECL_TYPE,
//};

#define GLEnumType(eEnumType) GL##eEnumType

#define DeclGLTypeMapFn(NUM, eEnumType) \
  extern const uint* GLEnumType(eEnumType##_);\
  \
  template<> \
  uint toGLType(eEnumType e) { \
  return GLEnumType(eEnumType##_)[e];\
  }\
  constexpr uint GLEnumType(eEnumType)[NUM+1] = 

#define GLTypeMapGuard(NUM, eEnumType) \
  const uint* GLEnumType(eEnumType##_) = GLEnumType(eEnumType);\
  static_assert(GLEnumType(eEnumType)[NUM] == NUM, "GL type map for type "#eEnumType" does not match the enum definition")


DeclGLTypeMapFn(NUM_DATA_DECL_TYPE, eDataDeclType) {
  GL_FLOAT,
  GL_BYTE,
  GL_UNSIGNED_BYTE,
  NUM_DATA_DECL_TYPE
};
GLTypeMapGuard(NUM_DATA_DECL_TYPE, eDataDeclType);

DeclGLTypeMapFn(NUM_DATA_DECL_TYPE, eDrawPrimitive) {
  GL_POINTS,
  GL_LINES,
  GL_TRIANGLES,
  NUM_PRIMITIVE_TYPES
};
GLTypeMapGuard(NUM_DATA_DECL_TYPE, eDrawPrimitive);

DeclGLTypeMapFn(NUM_COMPARE, eCompare) {
  GL_NEVER,
  GL_LESS,
  GL_LEQUAL,
  GL_GREATER,
  GL_GEQUAL,
  GL_EQUAL,
  GL_NOTEQUAL,
  GL_ALWAYS,
  NUM_COMPARE
};
GLTypeMapGuard(NUM_COMPARE, eCompare);

DeclGLTypeMapFn(NUM_CULL_MODE, eCullMode) {
  GL_BACK,
  GL_FRONT,
  0,
  NUM_CULL_MODE,
};
GLTypeMapGuard(NUM_CULL_MODE, eCullMode);

DeclGLTypeMapFn(NUM_FILL_MODE, eFillMode) {
  GL_FILL,
  GL_LINE,
  NUM_FILL_MODE,
};
GLTypeMapGuard(NUM_FILL_MODE, eFillMode);

DeclGLTypeMapFn(NUM_WIND_ORDER, eWindOrder) {
  GL_CW,
  GL_CCW,
  NUM_WIND_ORDER,
};
GLTypeMapGuard(NUM_WIND_ORDER, eWindOrder);

DeclGLTypeMapFn(NUM_BLEND_F, eBlendFactor) {
  GL_ONE,
  GL_ZERO,
  NUM_BLEND_F,
};
GLTypeMapGuard(NUM_BLEND_F, eBlendFactor);


DeclGLTypeMapFn(NUM_BLEND_OP, eBlendOp) {
    GL_FUNC_ADD,
    GL_FUNC_SUBTRACT,
    GL_FUNC_REVERSE_SUBTRACT,
    GL_MIN,
    GL_MAX,
    NUM_BLEND_OP,
};
GLTypeMapGuard(NUM_BLEND_OP, eBlendOp);


DeclGLTypeMapFn(NUM_TEXTURE_WRAP_MODE, eTextureWrapMode) {
  GL_CLAMP_TO_EDGE,
  GL_REPEAT,
  NUM_TEXTURE_WRAP_MODE,
};
GLTypeMapGuard(NUM_TEXTURE_WRAP_MODE, eTextureWrapMode);


DeclGLTypeMapFn(NUM_TEXTURE_SAMPLE_MODE, eTextureSampleMode) {
  GL_NEAREST,
  GL_LINEAR,
  NUM_TEXTURE_SAMPLE_MODE,
};
GLTypeMapGuard(NUM_TEXTURE_SAMPLE_MODE, eTextureSampleMode);
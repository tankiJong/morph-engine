//#include <windows.h>
//
#include <map>
#include <string>

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include <array>
#include "glFunctions.hpp"
#include "RenderBuffer.hpp"
#include "Engine/Renderer/Shader/ShaderProgram.hpp"
#include "Sampler.hpp"
#include "Engine/Application/Window.hpp"
#include "FrameBuffer.hpp"
#include "Sprite.hpp"
#include "Geometry/Mesh.hpp"
#include "Geometry/Vertex.hpp"
#include "Engine/Renderer/Shader/Shader.hpp"
#include "Geometry/Mesher.hpp"
#include "Engine/Renderer/Shader/Material.hpp"
#include "Engine/Renderer/Shader/ShaderPass.hpp"
#include "Engine/Renderer/RenderTarget.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Debug/Profile/Profiler.hpp"

#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

#undef near
#undef far

using Vertices = std::vector<vertex_pcu_t>;

Renderer::Renderer() {
}

Renderer::~Renderer() {
	for (const auto& kv: mTextures) {
		delete kv.second;
	}

  //QA: when to do the shutdown
  wglMakeCurrent(mHdc, nullptr);
  ::wglDeleteContext(mGlContext);
  ::ReleaseDC(mGlWnd, mHdc);

  mGlContext = nullptr;
  mHdc = nullptr;
  mGlWnd = nullptr;
  
  delete mDefaultShader;
  delete mDefaultCamera       ;
  delete mDefaultSampler      ;
  delete mDefaultDepthTarget  ;
  delete mDefaultColorTarget  ;

}

void Renderer::afterFrame() {
  // copies the default camera's framebuffer to the "null" framebuffer, 
  // also known as the back buffer.
  copyFrameBuffer(nullptr, mDefaultCamera->fbo());
	SwapBuffers(mHdc);
}

void Renderer::beforeFrame() {

  mUniformTime.putGpu();
  glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_TIME, mUniformTime.handle());
  GL_CHECK_ERROR();

}

void Renderer::setTexture(uint i, const Texture* texture) {
  if (texture == nullptr) {
    texture = createOrGetTexture("$");
  }

  // if (mCurrentTexture[i] == texture) return;

  glBindSampler(i, mDefaultSampler->handle());

  // Bind the texture
  mCurrentTexture[i] = texture;
  glActiveTexture(GL_TEXTURE0 + i);
  glBindTexture(GL_TEXTURE_2D, texture->getHandle());
}

void Renderer::drawLine(const vec3& start, const vec3& end, 
						const Rgba& startColor, const Rgba& endColor, float lineThickness) {
  setTexture(mTextures.at("$"));
  vertex_pcu_t verts[2] = {
    { start, startColor, {0,0}},
    { end, endColor, {0,1}}
  };
  glLineWidth(lineThickness);
  drawMeshImmediate(verts, 2, DRAW_LINES);
}

void Renderer::drawSprite(const vec3& position, const Sprite& sprite, mat44 orientation) {
  std::array<vec2, 4> bounds = sprite.bounds().vertices();
  std::array<vec2, 4> uvs = sprite.uv.vertices();
  // 0 1 2 0 2 3
  std::array<vertex_pcu_t, 6> mesh= {
    vertex_pcu_t{
      bounds[0], Rgba::white,  uvs[0]
    }, vertex_pcu_t{
      bounds[1], Rgba::white,  uvs[1]
    }, vertex_pcu_t{
      bounds[2], Rgba::white,  uvs[2]
    }, vertex_pcu_t{
      bounds[0], Rgba::white,  uvs[0]
    }, vertex_pcu_t{
      bounds[2], Rgba::white,  uvs[2]
    }, vertex_pcu_t{
      bounds[3], Rgba::white,  uvs[3]
    }
  };

  for(vertex_pcu_t& m: mesh) {
    m.position = (orientation * vec4(m.position, 0)).xyz();
    m.position += position;
  }

  setTexture(sprite.texture);
  drawMeshImmediate(mesh.data(), 6, DRAW_TRIANGES);
}

void Renderer::drawTexturedAABB2(const aabb2& bounds, 
								 const Texture& texture, 
								 const vec2& texCoordsAtMins, 
								 const vec2& texCoordsAtMaxs, 
								 const Rgba& tint) {
  setTexture(&texture);
  vertex_pcu_t verts[6] = {
    { bounds.mins, tint, texCoordsAtMins },
    { vec2{ bounds.maxs.x, bounds.mins.y }, tint, vec2{ texCoordsAtMaxs.x, texCoordsAtMins.y } },
    { bounds.maxs, tint, texCoordsAtMaxs },
    { bounds.mins, tint, texCoordsAtMins },
    { bounds.maxs, tint, texCoordsAtMaxs },
    { vec2{ bounds.mins.x, bounds.maxs.y }, tint, vec2{ texCoordsAtMins.x, texCoordsAtMaxs.y } },
  };

  drawMeshImmediate(verts, 6, DRAW_TRIANGES);
}

void Renderer::drawTexturedAABB2(const aabb2& bounds, const Texture& texture, 
                                 const aabb2& texCoords, const Rgba& tint) {
  drawTexturedAABB2(bounds, texture, texCoords.mins, texCoords.maxs, tint);
}

bool Renderer::init(HWND hwnd) {
  if(gGlLibrary == nullptr) {
    // load and get a handle to the opengl dll (dynamic link library)
    gGlLibrary = ::LoadLibraryA("opengl32.dll");
  }

  // Get the Device Context (DC) - how windows handles the interace to rendering devices
  // This "acquires" the resource - to cleanup, you must have a ReleaseDC(hwnd, hdc) call. 
  HDC hdc = ::GetDC(hwnd);

  // use the DC to create a rendering context (handle for all OpenGL state - like a pointer)
  // This should be very simiilar to SD1
  HGLRC tempContext = createOldRenderContext(hdc);

  ::wglMakeCurrent(hdc, tempContext);
  bindNewWGLFunctions();  // find the functions we'll need to create the real context; 

                          // create the real context, using opengl version 4.2
  HGLRC real_context = createRealRenderContext(hdc, 4, 2);

  // Set and cleanup
  ::wglMakeCurrent(hdc, real_context);
  ::wglDeleteContext(tempContext);

  // Bind all our OpenGL functions we'll be using.
  bindGLFunctions();

  // set the globals
  mGlWnd = hwnd;
  mHdc = hdc;
  mGlContext = real_context;

  postInit();

  return true;
}

void Renderer::postInit() {
  GL_CHECK_ERROR();

  mTextures["$"] = new Texture(Image(&Rgba::white, 1,1));
  // default_vao is a GLuint member variable
  glGenVertexArrays(1, &mDefaultVao);
  glBindVertexArray(mDefaultVao);

  mDefaultShader = new Shader();
  ShaderPass* pass = new ShaderPass();
  pass->prog() = createOrGetShaderProgram("@default");
  mDefaultShader->add(*pass);

  mDefaultSampler = new Sampler();

  setTexture("$");

  aabb2 bounds = Window::Get()->bounds();

  // create our output textures
  mDefaultColorTarget = createRenderTarget((uint)bounds.width(), (uint)bounds.height());
  mDefaultDepthTarget = createRenderTarget((uint)bounds.width(), (uint)bounds.height(),
                                              TEXTURE_FORMAT_D24S8);

  // setup the initial camera
  mDefaultCamera = new Camera();
  mDefaultCamera->setColorTarget(mDefaultColorTarget);
  mDefaultCamera->setDepthStencilTarget(mDefaultDepthTarget);

  // set our default camera to be our current camera
  setCamera(nullptr);

  mUniformTime.set(uniform_time_t());
  mUniformLights.set(light_buffer_t());
}

void Renderer::setState(const render_state& state) {
  if(state.cullMode != CULL_NONE) {
    glEnable(GL_CULL_FACE);
    glCullFace(toGLType(state.cullMode));
  } else {
    glDisable(GL_CULL_FACE);
  }

  glPolygonMode(GL_FRONT_AND_BACK, toGLType(state.fillMode));

  glFrontFace(toGLType(state.frontFace));

  enableDepth(state.depthMode, state.isWriteDepth);

  if (state.colorBlendOp == BLEND_OP_DISABLE || state.alphaBlendOp == BLEND_OP_DISABLE) {
    glDisable(GL_BLEND);
  } else {
    glEnable(GL_BLEND);
    glBlendEquationSeparate(toGLType(state.colorBlendOp), toGLType(state.alphaBlendOp));
    glBlendFuncSeparate(toGLType(state.colorSrcFactor), toGLType(state.colorDstFactor),
                        toGLType(state.alphaSrcFactor), toGLType(state.alphaDstFactor));
  }

}

void Renderer::setShader(const Shader* shader, uint passIndex) {
  
  const ShaderPass* pass = shader == nullptr ? mDefaultShader->pass(0) : &shader->pass(passIndex);
  
  glUseProgram(pass->prog()->handle());

  setState(pass->state());
}

void Renderer::setSampler(uint i, const Sampler* sampler) {
  glBindSampler(i, sampler->handle());
}

template<>
void Renderer::setUnifrom(const char* name, const float& value) {
  GLint prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
  GLint loc = glGetUniformLocation(prog, name);
  if(loc >= 0) {
    glUniform1f(loc, value);
  }
}

template<>
void Renderer::setUnifrom(const char* name, const vec2& value) {
  GLint prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
  GLint loc = glGetUniformLocation(prog, name);
  if (loc >= 0) {
    glUniform2fv(loc, 1, &value.x);
  }
}

template<>
void Renderer::setUnifrom(const char* name, const vec3& value) {
  GLint prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
  GLint loc = glGetUniformLocation(prog, name);
  if (loc >= 0) {
    glUniform3fv(loc, 1, &value.x);
  }
}

template<>
void Renderer::setUnifrom(const char* name, const Rgba& value) {
  GLint prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
  GLint loc = glGetUniformLocation(prog, name);
  if (loc >= 0) {
    vec4 scaledColor = value.normalized();
    glUniform4fv(loc, 1, scaledColor.data);
  }
}

template<>
void Renderer::setUnifrom(const char* name, const vec4& value) {
  GLint prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
  GLint loc = glGetUniformLocation(prog, name);
  if (loc >= 0) {
    glUniform4fv(loc, 1, value.data);
  }
}

template<>
void Renderer::setUnifrom(const char* name, const mat44& value) {
  GLint prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
  GLint loc = glGetUniformLocation(prog, name);
  if (loc >= 0) {
    glUniform1fv(loc, 16, value.data);
  }
}

void Renderer::setUniformBuffer(eUniformSlot slot, UniformBuffer& ubo) {
  ubo.putGpu();
  glBindBufferBase(GL_UNIFORM_BUFFER, slot, ubo.handle());
}

void Renderer::updateTime(float gameDeltaSec, float sysDeltaSec) {
  uniform_time_t* t = mUniformTime.as<uniform_time_t>();

  t->gameDeltaSeconds = gameDeltaSec;
  t->gameSeconds += gameDeltaSec;
  t->sysDeltaSeconds = sysDeltaSec;
  t->sysSeconds += sysDeltaSec;
}

void Renderer::drawText2(std::string_view text, float size, const Font* font, const vec3& up) {
  Mesher ms;

  ms.begin(DRAW_TRIANGES)
    .text(text, size, font, up)
    .end();

  Mesh* mesh = ms.createMesh<vertex_pcu_t>();
  setMaterial(Resource<Material>::get("material/font_default").get());
  setTexture(font->texture());

  drawMesh(*mesh);

  delete mesh;
}

Renderer* Renderer::Get() {
  return Engine::Get().mRenderer;
}

void Renderer::clearDepth(float depth) {
  enableDepth(COMPARE_LEQUAL, true);
  glClearDepthf(depth);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::enableDepth(eCompare compare, bool shouldWrite) {
  // enable/disable the dest
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(toGLType(compare));

  // enable/disable write
  glDepthMask(shouldWrite ? GL_TRUE : GL_FALSE);
}

void Renderer::disableDepth() {
  enableDepth(COMPARE_ALWAYS, false);
}

void Renderer::disableLight() {
  for(light_info_t& light: mUniformLights.as<light_buffer_t>()->lights) {
    light.color.a = 0.f;
  }
}

void Renderer::disableLight(uint index) {
  EXPECTS(index < NUM_MAX_LIGHTS);
  mUniformLights.as<light_buffer_t>()->lights[index].color.a = 0.f;
}

void Renderer::setCamera(Camera* camera) {
  if (camera == nullptr) {
    camera = mDefaultCamera;
  }

  camera->finalize(); // make sure the framebuffer is finished being setup; 

  glViewport(0, 0, camera->width(), camera->height());
  mCurrentCamera = camera;
}

void Renderer::resetAlphaBlending() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::setTexture(const Texture* texture) {
  setTexture(0, texture);
}

void Renderer::setSampler(const Sampler* sampler) {
  setSampler(0, sampler);
}

void Renderer::clearColor(const Rgba& color) {
  vec4 col = color.normalized();
  glClearColor(col.r, col.g, col.b, col.a);
  glClear(GL_COLOR_BUFFER_BIT);
  
}

void Renderer::setTexture(const char* path) {
  setTexture(createOrGetTexture(path));
}

void Renderer::setTexture(uint i, const char* path) {
  setTexture(i, createOrGetTexture(path));
}

HGLRC Renderer::createRealRenderContext(HDC hdc, int major, int minor) {
  // So similar to creating the temp one - we want to define 
  // the style of surface we want to draw to.  But now, to support
  // extensions, it takes key_value pairs
  int const format_attribs[] = {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,    // The rc will be used to draw to a window
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,    // ...can be drawn to by GL
    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,     // ...is double buffered
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // ...uses a RGBA texture
    WGL_COLOR_BITS_ARB, 24,             // 24 bits for color (8 bits per channel)
                                        // WGL_DEPTH_BITS_ARB, 24,          // if you wanted depth a default depth buffer...
                                        // WGL_STENCIL_BITS_ARB, 8,         // ...you could set these to get a 24/8 Depth/Stencil.
                                        NULL, NULL,                         // Tell it we're done.
  };

  // Given the above criteria, we're going to search for formats
  // our device supports that give us it.  I'm allowing 128 max returns (which is overkill)
  size_t const MAX_PIXEL_FORMATS = 128;
  int formats[MAX_PIXEL_FORMATS];
  int pixel_format = 0;
  UINT format_count = 0;

  BOOL succeeded = wglChoosePixelFormatARB(hdc,
                                           format_attribs,
                                           nullptr,
                                           MAX_PIXEL_FORMATS,
                                           formats,
                                           (UINT*)&format_count);

  if (!succeeded) {
    return NULL;
  }

  // Loop through returned formats, till we find one that works
  for (UINT i = 0; i < format_count; ++i) {
    pixel_format = formats[i];
    succeeded = SetPixelFormat(hdc, pixel_format, NULL); // same as the temp context; 
    if (succeeded) {
      break;
    } else {
      DWORD error = GetLastError();
      DebuggerPrintf("Failed to set the format: %u", error);
    }
  }

  if (!succeeded) {
    return NULL;
  }

  // Okay, HDC is setup to the rihgt format, now create our GL context

  // First, options for creating a debug context (potentially slower, but 
  // driver may report more useful errors). 
  int context_flags = 0;
#if defined(_DEBUG)
  context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

  // describe the context
  int const attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, major,                             // Major GL Version
    WGL_CONTEXT_MINOR_VERSION_ARB, minor,                             // Minor GL Version
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,   // Restrict to core (no compatibility)
    WGL_CONTEXT_FLAGS_ARB, context_flags,                             // Misc flags (used for debug above)
    0, 0
  };

  // Try to create context
  HGLRC context = wglCreateContextAttribsARB(hdc, NULL, attribs);
  if (context == NULL) {
    return NULL;
  }

  return context;
}

HGLRC Renderer::createOldRenderContext(HDC hdc) {
  // Setup the output to be able to render how we want
  // (in our case, an RGBA (4 bytes per channel) output that supports OpenGL
  // and is double buffered
  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 0; // 24; Depth/Stencil handled by FBO
  pfd.cStencilBits = 0; // 8; DepthStencil handled by FBO
  pfd.iLayerType = PFD_MAIN_PLANE; // ignored now according to MSDN

                                   // Find a pixel format that matches our search criteria above. 
  int pixel_format = ::ChoosePixelFormat(hdc, &pfd);
  if (pixel_format == NULL) {
    return NULL;
  }

  // Set our HDC to have this output. 
  if (!::SetPixelFormat(hdc, pixel_format, &pfd)) {
    return NULL;
  }

  // Create the context for the HDC
  HGLRC context = wglCreateContext(hdc);
  if (context == NULL) {
    return NULL;
  }

  // return the context; 
  return context;
}

Image Renderer::screenShot() {
  int x = mDefaultColorTarget->mDimensions.x, y = mDefaultColorTarget->mDimensions.y;
  Rgba* data = new Rgba[x * y];

  glBindFramebuffer(GL_READ_FRAMEBUFFER, mDefaultCamera->getFrameBufferHandle());
  glReadBuffer(GL_COLOR_ATTACHMENT0);

  glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, NULL);

  Image img(data, (uint)x, (uint)y);

  delete []data;

  return img;
}

void Renderer::setAmbient(const Rgba& color, float intensity) {
  mUniformLights.as<light_buffer_t>()->ambience = color.normalized();
  mUniformLights.as<light_buffer_t>()->ambience.a = intensity;
}

void Renderer::setAmbient(const vec4 ambience) {
  mUniformLights.as<light_buffer_t>()->ambience = ambience;
}

void Renderer::setLight(uint index, const light_info_t& lightInfo) {
  EXPECTS(index < NUM_MAX_LIGHTS);

  mUniformLights.as<light_buffer_t>()->lights[index] = lightInfo;
}

void Renderer::setDirectionalLight(uint        index,
                                   const vec3& position,
                                   const vec3& direction,
                                   float       intensity,
                                   const vec3& attenuation,
                                   const Rgba& color) {
  setSpotLight(index, position, direction, 90.f, 90.f, intensity, attenuation, color);
}

void Renderer::setPointLight(uint        index,
                             const vec3& position,
                             float       intensity,
                             const vec3& attenuation,
                             const Rgba& color) {
  EXPECTS(index < NUM_MAX_LIGHTS);

  mUniformLights.as<light_buffer_t>()->lights[index].asPointLight(position, intensity, attenuation, color);
}

void Renderer::setSpotLight(uint        index,
                            const vec3& position,
                            const vec3& direction,
                            float       innerAngle,
                            float       outerAngle,
                            float       intensity,
                            const vec3& attenuation,
                            const Rgba& color) {
  EXPECTS(index < NUM_MAX_LIGHTS);

  mUniformLights.as<light_buffer_t>()->lights[index].asSpotLight(position, direction, innerAngle, outerAngle, intensity, attenuation, color);
}

void Renderer::setModelMatrix(const mat44& model) {
  mUniformTransform.set(model);
  setUniformBuffer(UNIFORM_TRANSFORM, mUniformTransform);
}

void Renderer::setMaterial(const Material* material, uint passIndex) {
  TODO("handle nullptr, bind a default one");
  EXPECTS(material != nullptr);
  const Shader* shader = material->shader();

  setShader(shader, passIndex);

  for(MaterialProperty* prop: material->properties()) {
      // check whether is successful
    auto& info = shader->pass(passIndex).prog()->info();
    const PropertyBlockInfoBinding* binding = info.find(prop->name);

    if (binding == nullptr) continue;

    // location here is unifrom buffer location tbh
    uint loc = binding->bindInfo.location;
    if(loc != -1) {
      prop->bind(loc);
    }
  }

  for(MatProp<Texture>* tex: material->textures()) {
    if (tex == nullptr) continue;
    setTexture(tex->slot, tex->value);
    setSampler(tex->slot, tex->sampler);
  }
}

bool Renderer::copyFrameBuffer(FrameBuffer* dest, FrameBuffer* src) {

  // we need at least the src.
  if (src == nullptr) {
    return false;
  }

  // Get the handles - NULL refers to the "default" or back buffer FBO
  GLuint src_fbo = src->mHandle;
  GLuint dst_fbo = NULL;
  if (dest != nullptr) {
    dst_fbo = dest->mHandle;
  }

  // can't copy onto ourselves
  if (dst_fbo == src_fbo) {
    return false;
  }

  // the GL_READ_FRAMEBUFFER is where we copy from
  glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);

  // what are we copying to?
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo);

  // blit it over - get teh size
  // (we'll assume dst matches for now - but to be safe,
  // you should get dst_width and dst_height using either
  // dst or the window depending if dst was nullptr or not
  uint width = src->width();
  uint height = src->height();

  // Copy it over
  glBlitFramebuffer(0, 0, // src start pixel
                    width, height,        // src size
                    0, 0,                 // dst start pixel
                    width, height,        // dst size
                    GL_COLOR_BUFFER_BIT,  // what are we copying (just colour)
                    GL_NEAREST);         // resize filtering rule (in case src/dst don't match)

                                         // Make sure it succeeded

  // cleanup after ourselves
  glBindFramebuffer(GL_READ_FRAMEBUFFER, NULL);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

  return GLSucceeded();
}

void Renderer::drawAABB2(const aabb2& bounds, const Rgba& color, bool filled) {
  Mesher ms;
  if (filled) {
    ms.begin(DRAW_TRIANGES);
  } else {
    ms.begin(DRAW_LINES);
  }
  ms.color(color);
  ms.quad2(bounds, 0);
  ms.end();

  Mesh* immediateMesh = ms.createMesh<vertex_pcu_t>();
  drawMesh(*immediateMesh);

  delete immediateMesh;
}

void Renderer::drawCircle(const vec2& center, float radius, const Rgba& color, bool filled) {
  Vertices verts;
  verts.reserve(21);

  if(filled) {
    verts.emplace_back(center, color, vec2::zero);
  }

  for(int ii = 0; ii < 20; ii++) {
    float theta = 2.0f * PI * float(ii) * 0.05f;//get the current angle 

    float x = radius * cosf(theta);//calculate the x component 
    float y = radius * sinf(theta);//calculate the y component 

    verts.emplace_back(vec2{ x + center.x, y + center.y }, color, vec2::zero);
  }

    verts.emplace_back(vec2{ radius + center.x, center.y }, color, vec2::zero);
  if(!filled) {
    drawMeshImmediate(verts.data(), verts.size(), DRAW_LINES);
  } else {
    UNIMPLEMENTED();
    ERROR_AND_DIE("unimplemented");
  }
}

void Renderer::drawCube(const vec3& bottomCenter, const vec3& dimension, 
                        const Rgba& color, 
                        rect2 uvTop, rect2 uvSide, rect2 uvBottom) {
  float dx = dimension.x * .5f, dy = dimension.y * .5f, dz = dimension.z * .5f;
  std::array<vec3, 8> vertices = {
    bottomCenter + vec3{ -dx, 2.f * dy, -dz },
    bottomCenter + vec3{  dx, 2.f * dy, -dz },
    bottomCenter + vec3{  dx, 2.f * dy,  dz },
    bottomCenter + vec3{ -dx, 2.f * dy,  dz },

    bottomCenter + vec3{ -dx, 0, -dz },
    bottomCenter + vec3{  dx, 0, -dz },
    bottomCenter + vec3{  dx, 0,  dz },
    bottomCenter + vec3{ -dx, 0,  dz }
  };

  { // top
    auto uvs = uvTop.vertices();
    std::array<vertex_pcu_t, 6> mesh = {
      vertex_pcu_t(vertices[0], color, uvs[0]),
      vertex_pcu_t(vertices[1], color, uvs[1]),
      vertex_pcu_t(vertices[2], color, uvs[2]),

      vertex_pcu_t(vertices[0], color, uvs[0]),
      vertex_pcu_t(vertices[2], color, uvs[2]),
      vertex_pcu_t(vertices[3], color, uvs[3]),
    };

    drawMeshImmediate(mesh, DRAW_TRIANGES);
  }

  { // bottom
    auto uvs = uvBottom.vertices();
    std::array<vertex_pcu_t, 6> mesh = {
      vertex_pcu_t(vertices[4], color, uvs[0]),
      vertex_pcu_t(vertices[5], color, uvs[1]),
      vertex_pcu_t(vertices[6], color, uvs[2]),

      vertex_pcu_t(vertices[4], color, uvs[0]),
      vertex_pcu_t(vertices[6], color, uvs[2]),
      vertex_pcu_t(vertices[7], color, uvs[3]),
    };

    drawMeshImmediate(mesh, DRAW_TRIANGES);
  }

  // four sides
    auto uvs = uvSide.vertices();
  {
    std::array<uint, 6> indices = { 4, 5, 1, 4, 1,0 };
    for(uint i = 0; i<3; i++) {
      std::array<vertex_pcu_t, 6> mesh = {
        vertex_pcu_t(vertices[(indices[0] + i)%8], color, uvs[0]),
        vertex_pcu_t(vertices[(indices[1] + i)%8], color, uvs[1]),
        vertex_pcu_t(vertices[(indices[2] + i)%8], color, uvs[2]),
        vertex_pcu_t(vertices[(indices[3] + i)%8], color, uvs[0]),
        vertex_pcu_t(vertices[(indices[4] + i)%8], color, uvs[2]),
        vertex_pcu_t(vertices[(indices[5] + i)%8], color, uvs[3]),
      };

      drawMeshImmediate(mesh, DRAW_TRIANGES);
    }
  }

  {
    std::array<uint, 6> indices = { 7, 4, 0, 7, 0, 3 };
    std::array<vertex_pcu_t, 6> mesh = {
      vertex_pcu_t(vertices[indices[0]], color, uvs[0]),
      vertex_pcu_t(vertices[indices[1]], color, uvs[1]),
      vertex_pcu_t(vertices[indices[2]], color, uvs[2]),
      vertex_pcu_t(vertices[indices[3]], color, uvs[0]),
      vertex_pcu_t(vertices[indices[4]], color, uvs[2]),
      vertex_pcu_t(vertices[indices[5]], color, uvs[3]),
    };

    drawMeshImmediate(mesh, DRAW_TRIANGES);
  }
}

void Renderer::drawMesh(const Mesh& mesh) {
  PROF_FUNC();
  for(int i = 0; i<mesh.layout().attributes().size();i++) {
    glDisableVertexAttribArray(i);
  }

  setUniformBuffer(UNIFORM_LIGHT, mUniformLights);
  GLint prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
  for(const VertexAttribute& attribute: mesh.layout().attributes()) {
    GLint bindIdx = glGetAttribLocation(prog, attribute.name.c_str());

    if(bindIdx >= 0) {
      const VertexBuffer& vbo = mesh.vertices(attribute.streamIndex);
      glBindBuffer(GL_ARRAY_BUFFER, vbo.handle());

      glEnableVertexAttribArray(bindIdx);
      glVertexAttribPointer(bindIdx, 
                            attribute.count, 
                            toGLType(attribute.type), 
                            attribute.isNormalized ? GL_FALSE : GL_TRUE, 
                            vbo.vertexStride,
                            reinterpret_cast<GLvoid*>((u64)attribute.offset));
    }
  }
//  // position
//  GLint posBind = glGetAttribLocation(mCurrentShader->prog()->handle(), "POSITION");
//  if(posBind >= 0) {
//    glEnableVertexAttribArray(posBind);
//
//    glVertexAttribPointer(posBind, 3, GL_FLOAT, GL_FALSE, mesh.vertices().vertexStride, (GLvoid*)offsetof(vertex_pcu_t, position));
//  }

//  glUseProgram(mCurrentShader->pass(0).prog()->handle());
  
//  GLint loc = glGetUniformLocation(mCurrentShader->prog()->handle(), "PROJECTION");
//  if (loc >= 0) {
//    glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mCurrentCamera->mProjMatrix);
//  }

  static UniformBuffer* ubo = UniformBuffer::For(mCurrentCamera->ubo());

  ubo->set(mCurrentCamera->ubo());

  setUniformBuffer(UNIFORM_CAMERA, *ubo);


  glBindFramebuffer(GL_FRAMEBUFFER, mCurrentCamera->getFrameBufferHandle());
  for (const draw_instr_t& ins : mesh.instructions()) {
    if (ins.useIndices) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indices().handle());
//      glDrawRangeElements(toGLType(ins.prim), ins.startIndex, ins.startIndex + ins.elementCount - 1, ins.elementCount, GL_UNSIGNED_INT, 0);
//      GL_CHECK_ERROR();
      glDrawElements(toGLType(ins.prim), ins.elementCount, GL_UNSIGNED_INT, (void*)(ins.startIndex*sizeof(uint)));
    } else {
      glDrawArrays(toGLType(ins.prim), ins.startIndex, ins.elementCount);
    }
  }

}

void Renderer::drawMeshImmediate(const vertex_pcu_t* vertices, size_t numVerts, eDrawPrimitive drawPrimitive) {
  static Vertex v;
  VertexMesh<vertex_pcu_t> immediateMesh;

  v.clear();

  for(size_t i = 0; i<numVerts; i++) {
    const vertex_pcu_t& vert = vertices[i];
    v.push({ vert.position, vert.color, vert.uvs });
  }

  immediateMesh.setVertices(v);
  immediateMesh.pushInstruction(drawPrimitive, false, 0, v.count());

  drawMesh(immediateMesh);
}

void Renderer::cleanScreen(const Rgba& color) {
	float r = 0, g = 0 , b = 0, a = 1;
  color.getAsFloats(r, g, b, a);

  glClearColor(r,g,b,a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool Renderer::copyTexture(Texture* from, Texture* to) {
  static FrameBuffer src, dst;

  src.setColorTarget(from);
  src.finalize();

  if(to == nullptr) {
    return copyFrameBuffer(nullptr, &src);
  }

  dst.setColorTarget(to);
  dst.finalize();
  return copyFrameBuffer(&dst, &src);
}

bool Renderer::copyTexture(Texture* from, uint fromX, uint fromY, Texture* to, uint toX, uint toY, uint width, uint height) {
  static FrameBuffer src;

  if (from->mFormat != to->mFormat) return false;

  src.setDepthStencilTarget(from);
  src.finalize();

  glBindFramebuffer(GL_READ_FRAMEBUFFER, src.mHandle);

  uint internalformat;
  uint channels;
  uint layout;

  glBindTexture(GL_TEXTURE_2D, to->mTextureID);
  GLTexFormat(from->mFormat, internalformat, channels, layout);
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, toX, toY, fromX, fromY, width, height);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, NULL);

  return GLSucceeded();
}

Texture* Renderer::createOrGetTexture(const std::string& filePath) {
	auto it = mTextures.find(filePath);
	if (it == mTextures.end()) {
		Texture* texture = new Texture(filePath);
		mTextures[filePath] = texture;
		return texture;
	}
	return it->second;
}

RenderTarget* Renderer::createRenderTarget(uint width, uint height, eTextureFormat fmt) {
  return new RenderTarget(width, height, fmt);
}

ShaderProgram* Renderer::createOrGetShaderProgram(const char* nameWithPath) {
  std::string name = std::string(nameWithPath);
  auto it = mShaderPrograms.find(name);
  if (it != mShaderPrograms.end()) return it->second;

  ShaderProgram* program = new ShaderProgram();
  bool success = program->fromFile(nameWithPath, "TEST_ON_FLAG;COLOR_FLAG=1");

  if (!success) {
    program->fromFile("@invalid");
  };
  
  mShaderPrograms[name] = program;
  return program;
}
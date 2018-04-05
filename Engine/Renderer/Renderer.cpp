//#include <windows.h>
//
#include <map>
#include <string>

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "BitmapFont.hpp"
#include "SpriteSheet.hpp"
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

#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

#undef near
#undef far

using Vertices = std::vector<vertex_pcu_t>;

Renderer::Renderer() {
}

Renderer::~Renderer() {
  for (const auto& kv : mFonts) {
    delete kv.second;
  }

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
  
  delete mCurrentShaderProgram;
  delete mDefaultShaderProgram;
  delete mDefaultCamera       ;
  delete mDefaultSampler      ;
  delete mDefaultDepthTarget  ;
  delete mDefaultColorTarget  ;

}

void Renderer::afterFrame() {
  // copies the default camera's framebuffer to the "null" framebuffer, 
  // also known as the back buffer.
  copyFrameBuffer(nullptr, mDefaultCamera->mFrameBuffer);
	SwapBuffers(mHdc);
}

void Renderer::beforeFrame() {

  mUniformTime.putGpu();
  glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_TIME, mUniformTime.handle());
  GL_CHECK_ERROR();

}

void Renderer::bindTexture(uint i, const Texture* texture) {
  if (texture == nullptr) {
    texture = createOrGetTexture("$");
  }

  if (mCurrentTexture[i] == texture) return;

  glBindSampler(i, mDefaultSampler->getHandle());

  // Bind the texture
  mCurrentTexture[i] = texture;
  glActiveTexture(GL_TEXTURE0 + i);
  glBindTexture(GL_TEXTURE_2D, texture->getHandle());
}

void Renderer::drawLine(const vec3& start, const vec3& end, 
						const Rgba& startColor, const Rgba& endColor, float lineThickness) {
  bindTexture(mTextures.at("$"));
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

  bindTexture(sprite.texture);
  drawMeshImmediate(mesh.data(), 6, DRAW_TRIANGES);
}

void Renderer::drawTexturedAABB2(const aabb2& bounds, 
								 const Texture& texture, 
								 const vec2& texCoordsAtMins, 
								 const vec2& texCoordsAtMaxs, 
								 const Rgba& tint) {
  bindTexture(&texture);
  vertex_pcu_t verts[6] = {
    { bounds.mins, tint, texCoordsAtMins },
    { vec2{ bounds.maxs.x, bounds.mins.y }, tint, vec2{ texCoordsAtMaxs.x, texCoordsAtMins.y } },
    { bounds.maxs, tint, texCoordsAtMaxs },
    { bounds.mins, tint, texCoordsAtMins },
    { vec2{ bounds.mins.x, bounds.maxs.y }, tint, vec2{ texCoordsAtMins.x, texCoordsAtMaxs.y } },
    { bounds.maxs, tint, texCoordsAtMaxs },
  };

  drawMeshImmediate(verts, 6, DRAW_TRIANGES);
}

void Renderer::drawTexturedAABB2(const aabb2& bounds, const Texture& texture, 
                                 const aabb2& texCoords, const Rgba& tint) {
  drawTexturedAABB2(bounds, texture, texCoords.mins, texCoords.maxs, tint);
}

void Renderer::drawText2D(const vec2& drawMins, const std::string& asciiText, 
                          float cellHeight, const Rgba& tint, 
                          float aspectScale, const BitmapFont* font) {
  if(font == nullptr) {
    font = BitmapFont::getDefaultFont();
  }

  GUARANTEE_OR_DIE(font != nullptr, "no font assigned");

  float charWidth = font->getStringWidth(asciiText, cellHeight, aspectScale) / asciiText.length();
  vec2 dx(charWidth, 0.f), dy(0.f, cellHeight);

  for(int i =0; i<(int)asciiText.length(); i++) {
    vec2 mins = drawMins + float(i) * dx;
    aabb2 textBounds(mins, mins + dx + dy);
    aabb2 uv = font->getUVsForGlyph(asciiText[i]);
    drawTexturedAABB2(textBounds, font->m_spriteSheet.getTexture(), uv, tint);
  }
}

void Renderer::drawText2D(const vec2& drawMins, const std::string& asciiText, float cellHeight, const BitmapFont* font, const Rgba& tint, float aspectScale) {
  drawText2D(drawMins, asciiText, cellHeight, tint, aspectScale, font);
}

void Renderer::drawText2D(const vec2& drawMins, 
                          const std::vector<std::string>& asciiTexts, float cellHeight, 
                          const std::vector<Rgba>& tints, const BitmapFont* font, 
                          float aspectScale) {
  EXPECTS(asciiTexts.size() == tints.size());
  if (font == nullptr) {
    font = BitmapFont::getDefaultFont();
  }

  GUARANTEE_OR_DIE(font != nullptr, "no font assigned");

  float charWidth = 0;
  std::string t;
  t.reserve(asciiTexts.size() * 100);
  for(auto& asciiText: asciiTexts) {
    t.append(asciiText);
  }
  charWidth += font->getStringWidth(t, cellHeight, aspectScale) / t.length();

  vec2 dx(charWidth, 0.f), dy(0.f, cellHeight);
  uint currentChIndex = 0;
  for(uint j = 0, size = asciiTexts.size(); j < size; j++) {
    const std::string& asciiText = asciiTexts[j];
    const Rgba& tint = tints[j];
    for (uint i = 0; i<asciiText.length(); i++) {
      vec2 mins = drawMins + float(currentChIndex++) * dx;
      aabb2 textBounds(mins, mins + dx + dy);
      aabb2 uv = font->getUVsForGlyph(asciiText[i]);
      drawTexturedAABB2(textBounds, font->m_spriteSheet.getTexture(), uv, tint);
    }
  }
}

void Renderer::drawTextInBox2D(const aabb2& bounds, const std::string& asciiText, float cellHeight, vec2 aligns, eTextDrawMode drawMode, const BitmapFont* font, const Rgba& tint, float aspectScale) {
  auto texts = split(asciiText.c_str(), "\n");
  std::vector<std::string>* toDraw = nullptr;
  std::vector<std::string> blocks = {};

  if(drawMode == TEXT_DRAW_SHRINK_TO_FIT) {
    float scale = 1.f;
    const std::string& longest 
      = *std::max_element(texts.begin(), texts.end(),
                          [&font = font, &cellHeight = cellHeight](auto& a, auto& b) {
                            return font->getStringWidth(a, cellHeight) < font->getStringWidth(b, cellHeight);
                          });

    float scaleX = 1.f, scaleY = 1.f;

    float textWidth = font->getStringWidth(longest, cellHeight);
    if( textWidth > bounds.width()) {
      scaleX = bounds.width() / textWidth;
    }

    float textHeight = cellHeight * texts.size();
    if(textHeight > bounds.height()) {
      scaleY = bounds.height() / textHeight;
    }

    scale = std::min<float>(scaleY, scaleX);
    toDraw = &texts;
    cellHeight *= scale;
    goto STEP_DRAW;
  }

  if(drawMode == TEXT_DRAW_OVERRUN) {
    toDraw = &texts;
    goto STEP_DRAW;
  }


  // drawMode == TEXT_DRAW_WORD_WRAP
  {
    const int numMaxChar = font->maxCharacterInWidth(bounds.width(), cellHeight);
    for(const auto& line: texts) {
      if(line.size() > unsigned int(numMaxChar)) {
        unsigned int startPos = 0;

//        while(line[startDrawingPos] == ' ') {
//          startDrawingPos++;
//        }
        while(startPos < line.size()) {
          const int origin = std::min<int>(startPos + numMaxChar, static_cast<int>(line.size()));
          unsigned int endPos = origin;
          bool isFound = false;
          for(; endPos >= startPos; endPos--) {
            if (line[endPos] == ' ') {
              isFound = true;
              break;
            }
          }
          if(!isFound) {
            endPos = origin;
            for ( ; endPos < line.size(); endPos++) {
              if (line[endPos] == ' ') {
                isFound = true;
                break;
              }
            }
          }
          if(isFound) {
            blocks.push_back(line.substr(startPos, endPos - startPos));
            startPos = endPos + 1;
          } else {
            blocks.push_back(line.substr(startPos, line.size() - startPos));
            break;
          }
//          while (startDrawingPos < line.size() && line[startDrawingPos] == ' ') {
//            startDrawingPos++;
//          }
        }
      } else {
        blocks.push_back(line);
      }
    }

    toDraw = &blocks;    
  }

STEP_DRAW:
  const auto& longest = *std::max_element(toDraw->begin(), toDraw->end(), [](std::string& a, std::string& b) { return a.size() < b.size(); });

  float blockWidth = font->getStringWidth(longest, cellHeight);
  float blockHeight = cellHeight * toDraw->size();

  vec2 anchor(bounds.mins.x, bounds.maxs.y);

  vec2 padding = bounds.getDimensions() - vec2(blockWidth, blockHeight);
  padding.x *= aligns.x;
  padding.y *= -aligns.y;

  anchor += padding;

  anchor.y -= cellHeight;

  for(const auto& line: *toDraw) {
    drawText2D(anchor, line, cellHeight, tint, aspectScale, font);
    anchor.y -= cellHeight;
  }
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

  mDefaultShaderProgram = createOrGetShaderProgram("@default");
  mCurrentShaderProgram = mDefaultShaderProgram;

  mDefaultSampler = new Sampler();

  setTexture("$");

  aabb2 bounds = Window::getInstance()->bounds();

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
}

void Renderer::setOrtho2D(const vec2& bottomLeft, const vec2& topRight) {
  mCurrentCamera->mProjMatrix = mat44::makeOrtho2D(bottomLeft, topRight);
}


void Renderer::setOrtho(float width, float height, float near, float far) {
  mCurrentCamera->mProjMatrix = mat44::makeOrtho(width, height, near, far);
}

void Renderer::setProjection(const mat44& projection) {
  mCurrentCamera->mProjMatrix = projection;
}

void Renderer::setSampler(uint i, Sampler* sampler) {
  glBindSampler(i, sampler->getHandle());
}

void Renderer::setUniformBuffer(eUniformUnit slot, UniformBuffer& ubo) {
  ubo.putGpu();
  glBindBufferBase(GL_UNIFORM_BUFFER, slot, ubo.handle());
}

void Renderer::pushMatrix() {
//	glPushMatrix();
  UNIMPLEMENTED();
}

void Renderer::popMatrix() {
//	glPopMatrix();
  UNIMPLEMENTED();
}

void Renderer::traslate2D(const vec2& translation) {
//	glTranslatef(translation.x, translation.y, 0);
  UNIMPLEMENTED();
}

void Renderer::updateTime(float gameDeltaSec, float sysDeltaSec) {
  uniform_time_t* t = mUniformTime.as<uniform_time_t>();

  t->gameDeltaSeconds = gameDeltaSec;
  t->gameSeconds += gameDeltaSec;
  t->sysDeltaSeconds = sysDeltaSec;
  t->sysSeconds += sysDeltaSec;
}

void Renderer::useShaderProgram(ShaderProgram* program) {
  mCurrentShaderProgram = program == nullptr ? mShaderPrograms.at("@default"): program;
  ENSURES(mCurrentShaderProgram != nullptr);
}

void Renderer::clearDepth(float depth) {
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

void Renderer::setAddtiveBlending() {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void Renderer::setCamera(Camera* camera) {
  if (camera == nullptr) {
    camera = mDefaultCamera;
  }

  camera->finalize(); // make sure the framebuffer is finished being setup; 
  mCurrentCamera = camera;
}

void Renderer::resetAlphaBlending() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool Renderer::reloadShaderProgram() {
  bool success = true;

  for(auto& kv: mShaderPrograms) {
    success = success & kv.second->fromFile(kv.first.c_str());
  }

  return success;
}

bool Renderer::reloadShaderProgram(const char* nameWithPath) {
  std::string name = std::string(nameWithPath);
  auto it = mShaderPrograms.find(name);

  EXPECTS(it != mShaderPrograms.end());

  return it->second->fromFile(nameWithPath);
}

void Renderer::bindTexture(const Texture* texture) {
  bindTexture(0, texture);
}

void Renderer::bindSampler(Sampler* sampler) {
  mDefaultSampler = sampler;
}

void Renderer::setTexture(const char* path) {
  bindTexture(createOrGetTexture(path));
}

void Renderer::setTexture(uint i, const char* path) {
  bindTexture(i, createOrGetTexture(path));
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

void Renderer::rotate2D(float degree) {
//	glRotatef(degree, 0.f, 0.f, 1);
  UNIMPLEMENTED();
}

Image Renderer::screenShot() {
  int x = mDefaultColorTarget->mDimensions.x, y = mDefaultColorTarget->mDimensions.y;
  Rgba* data = new Rgba[x * y];

  glBindFramebuffer(GL_READ_FRAMEBUFFER, mDefaultCamera->getFrameBufferHandle());
  GL_CHECK_ERROR();
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  GL_CHECK_ERROR();

  glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);
  GL_CHECK_ERROR();

  glBindFramebuffer(GL_READ_FRAMEBUFFER, NULL);
  GL_CHECK_ERROR();

  Image img(data, (uint)x, (uint)y);

  delete []data;

  return img;
}

void Renderer::scale2D(float ratioX, float ratioY, float ratioZ) {
//	glScalef(ratioX, ratioY, ratioZ);
  UNIMPLEMENTED();
}

void Renderer::swapBuffers(HDC ctx) {
	SwapBuffers(ctx);
}

bool Renderer::copyFrameBuffer(FrameBuffer* dest, FrameBuffer* src) {
  GL_CHECK_ERROR();

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
  GL_CHECK_ERROR();

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

void Renderer::loadIdentity() {
//	glLoadIdentity();
  UNIMPLEMENTED();
}

void Renderer::drawAABB2(const aabb2& bounds, const Rgba& color, bool filled) {
  if (filled) {
    vertex_pcu_t verts[6] = {
      { bounds.mins, color, vec2::zero },
      { vec2{ bounds.maxs.x, bounds.mins.y }, color, vec2::right },
      { bounds.maxs, color, vec2::one },

      { bounds.mins, color, vec2::zero },
      { bounds.maxs, color, vec2::one },
      { vec2{ bounds.mins.x, bounds.maxs.y }, color, vec2::top },
    };
    drawMeshImmediate(verts, 6, DRAW_TRIANGES);
  } else {
    auto vertices = bounds.vertices();
    vertex_pcu_t verts[8] = {
      { vec3(vertices[0]), color, vec2{ 0,0 } },
      { vec3(vertices[1]), color,vec2{ 0,0 } },
      { vec3(vertices[1]), color,vec2{ 0,0 } },
      { vec3(vertices[2]), color,vec2{ 0,0 } },
      { vec3(vertices[2]), color,vec2{ 0,0 } },
      { vec3(vertices[3]), color, vec2{ 0,0 } },
      { vec3(vertices[0]), color, vec2{ 0,0 } },
    };
    drawMeshImmediate(verts, 4, DRAW_LINES);
  }
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
  GL_CHECK_ERROR();

  for(const VertexAttribute& attribute: mesh.layout().attributes()) {
    GLint bindIdx = glGetAttribLocation(mCurrentShaderProgram->handle(), attribute.name.c_str());

    if(bindIdx >= 0) {
      const VertexBuffer& vbo = mesh.vertices(attribute.streamIndex);
      glBindBuffer(GL_ARRAY_BUFFER, vbo.handle());

      glEnableVertexAttribArray(bindIdx);
      glVertexAttribPointer(bindIdx, 
                            attribute.count, 
                            toGLType(attribute.type), 
                            attribute.isNormalized ? GL_FALSE : GL_TRUE, 
                            vbo.vertexStride,
                            (GLvoid*)attribute.offset);
    }
  }
//  // position
//  GLint posBind = glGetAttribLocation(mCurrentShaderProgram->handle(), "POSITION");
//  if(posBind >= 0) {
//    glEnableVertexAttribArray(posBind);
//
//    glVertexAttribPointer(posBind, 3, GL_FLOAT, GL_FALSE, mesh.vertices().vertexStride, (GLvoid*)offsetof(vertex_pcu_t, position));
//  }
//
//  // color
//  GLint colorBind = glGetAttribLocation(mCurrentShaderProgram->handle(), "COLOR");
//  if(colorBind >= 0) {
//    glEnableVertexAttribArray(colorBind);
//    glVertexAttribPointer(colorBind, 3, GL_UNSIGNED_BYTE, GL_TRUE, mesh.vertices().vertexStride, (GLvoid*)offsetof(vertex_pcu_t, color));
//  }
//
//  // uv
//  GLint uvBind = glGetAttribLocation(mCurrentShaderProgram->handle(), "UV");
//  if(uvBind >= 0) {
//    glEnableVertexAttribArray(uvBind);
//    glVertexAttribPointer(uvBind, 2, GL_FLOAT, GL_FALSE, mesh.vertices().vertexStride, (GLvoid*)offsetof(vertex_pcu_t, uvs));
//  }
  glUseProgram(mCurrentShaderProgram->handle());
  
//  GLint loc = glGetUniformLocation(mCurrentShaderProgram->handle(), "PROJECTION");
//  if (loc >= 0) {
//    glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mCurrentCamera->mProjMatrix);
//  }
//
//  loc = glGetUniformLocation(mCurrentShaderProgram->handle(), "VIEW");
//  if (loc >= 0) {
//    glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mCurrentCamera->mViewMatrix);
//  }

  static UniformBuffer* ubo = UniformBuffer::For(mCurrentCamera->cameraBlock);

  ubo->set(mCurrentCamera->cameraBlock);

  setUniformBuffer(UNIFORM_CAMERA, *ubo);
  const draw_instr_t& ins = mesh.instruction();


  glBindFramebuffer(GL_FRAMEBUFFER, mCurrentCamera->getFrameBufferHandle());

  if(ins.useIndices) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indices().handle());
    glDrawElements(toGLType(ins.prim), ins.elementCount, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(toGLType(ins.prim), ins.startIndex, ins.elementCount);
  }

}

void Renderer::drawMeshImmediate(const vertex_pcu_t* vertices, size_t numVerts, eDrawPrimitive drawPrimitive) {
  // first, copy the memory to the buffer
  mTempRenderBuffer.copyToGpu(sizeof(vertex_pcu_t) * numVerts, vertices);
  
  //--------------------------bind position---------------------------------

  // Describe the buffer - first, figure out where the shader is expecting
  // position to be.
  GLint posBind = glGetAttribLocation(mCurrentShaderProgram->handle(), "POSITION");

  // Next, bind the buffer we want to use; 
  glBindBuffer(GL_ARRAY_BUFFER, mTempRenderBuffer.handle());

  // next, bind where position is in our buffer to that location; 
  if (posBind >= 0) {
    // enable this location
    glEnableVertexAttribArray(posBind);

    // describe the data
    glVertexAttribPointer(posBind, // where?
                          3,                           // how many (vec3 has 3 floats)
                          GL_FLOAT,                    // type? (vec3 is 3 floats)
                          GL_FALSE,                    // Should data be normalized
                          sizeof(vertex_pcu_t),              // stride (how far between each vertex)
                          (GLvoid*)offsetof(vertex_pcu_t, position)); // From the start of a vertex, where is this data?
  }
  //--------------------------bind UV---------------------------------

  GLint uvBind = glGetAttribLocation(mCurrentShaderProgram->handle(), "UV");
  glBindBuffer(GL_ARRAY_BUFFER, mTempRenderBuffer.handle());

  if (uvBind >= 0) {
    glEnableVertexAttribArray(uvBind);
    glVertexAttribPointer(uvBind,
                          2,                           
                          GL_FLOAT,                    
                          GL_FALSE,                   
                          sizeof(vertex_pcu_t),
                          (GLvoid*)offsetof(vertex_pcu_t, uvs));
  }

  //-------------------------Texture---------------------------------
  // bind in bindTexture

  //-------------------------Color-----------------------------------
  // Next, bind the buffer we want to use;
  glBindBuffer(GL_ARRAY_BUFFER, mTempRenderBuffer.handle());

  // next, bind where position is in our buffer to that location;
  GLint bind = glGetAttribLocation(mCurrentShaderProgram->handle(), "COLOR");
  if (bind >= 0) {
    // enable this location
    glEnableVertexAttribArray(bind);

    // describe the data
    glVertexAttribPointer(bind, // where?
                          4,                           // how many (RGBA is 4 unsigned chars)
                          GL_UNSIGNED_BYTE,            // type? (RGBA is 4 unsigned chars)
                          GL_TRUE,                     // Normalize components, maps 0-255 to 0-1.
                          sizeof(vertex_pcu_t),              // stride (how far between each vertex)
                          (GLvoid*)offsetof(vertex_pcu_t, color)); // From the start of a vertex, where is this data?
  }

  // Now that it is described and bound, draw using our program
  glUseProgram(mCurrentShaderProgram->handle());

  //-----------------------Matrix------------------------------------
  GLint loc = glGetUniformLocation(mCurrentShaderProgram->handle(), "PROJECTION");
  if (loc >= 0) {
    // you "may" need to use GL_TRUE, depending on your matrix layout
    // and whether you prefer to multiply left or right;
    // acts on the currently bound program (glUseProgram)
    glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mCurrentCamera->mProjMatrix);
  }

  loc = glGetUniformLocation(mCurrentShaderProgram->handle(), "VIEW");
  if (loc >= 0) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mCurrentCamera->mViewMatrix);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, mCurrentCamera->getFrameBufferHandle());
  glDrawArrays(toGLType(drawPrimitive), 0, numVerts);

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

BitmapFont* Renderer::createOrGetBitmapFont(const char* bitmapFontName, const char* path) {
  const char* fullPath = std::string(path).append(bitmapFontName).c_str();
  return createOrGetBitmapFont(fullPath);
}

BitmapFont* Renderer::createOrGetBitmapFont(const char* fontNameWithPath) {
  auto kv = mFonts.find(fontNameWithPath);
  if (kv != mFonts.end()) {
    return kv->second;
  }

  Texture* fontTex = createOrGetTexture(fontNameWithPath);
  BitmapFont* font = new BitmapFont(fontNameWithPath, *(new SpriteSheet(*fontTex, 16, 16)));
  mFonts[fontNameWithPath] = font;

  return font;
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

Texture* Renderer::createRenderTarget(uint width, uint height, eTextureFormat fmt) {
  Texture *tex = new Texture();
  tex->setupRenderTarget(width, height, fmt);
  return tex;
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

bool Renderer::applyEffect(ShaderProgram* program) {
  if(mEffectTarget == nullptr) {
    mEffectTarget = mDefaultColorTarget;
    if(mEffectScratch == nullptr) {
      mEffectScratch = mEffectTarget->clone();
    }
  }

  if(mEffectCamera == nullptr) {
    mEffectCamera = new Camera();
  }

  mEffectCamera->setColorTarget(mEffectScratch);

  setCamera(mEffectCamera);

  useShaderProgram(program);

  bindTexture(0, mEffectTarget);
  
  bindTexture(1, mDefaultDepthTarget);

  drawAABB2({ {-1.f, -1.f}, {1.f, 1.f} }, Rgba::white);
  GL_CHECK_ERROR();

  return copyTexture(mEffectScratch, mDefaultColorTarget);
}
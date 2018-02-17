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
#include "ShaderProgram.hpp"
#include "Sampler.hpp"
#include "Engine/Application/Window.hpp"
#include "FrameBuffer.hpp"

#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

#undef near
#undef far

int g_openGlPrimitiveTypes[NUM_PRIMITIVE_TYPES] =
{
  GL_POINTS,			// called PRIMITIVE_POINTS		in our engine
  GL_LINES,			// called PRIMITIVE_LINES		in our engine
  GL_LINE_LOOP,
  GL_TRIANGLES,		// called PRIMITIVE_TRIANGES	in our engine
  GL_TRIANGLE_FAN,
  GL_QUADS			// called PRIMITIVE_QUADS		in our engine
};

uint g_openGLCompare[NUM_COMPARE] = {
  GL_NEVER,
  GL_LESS,
  GL_LEQUAL,
  GL_GREATER,
  GL_GEQUAL,
  GL_EQUAL,
  GL_NOTEQUAL,
  GL_ALWAYS,
};

using Vertices = std::vector<Vertex_PCU>;

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
  delete mCurrentCamera       ;
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
//  glClearColor(1.f, 0, 0, 1);
  cleanScreen(Rgba::gray);
}

void Renderer::drawLine(const vec3& start, const vec3& end, 
						const Rgba& startColor, const Rgba& endColor, float lineThickness) {
  bindTexutre(mTextures.at("$"));
  Vertex_PCU verts[2] = {
    { start, startColor, {0,0}},
    { end, endColor, {0,1}}
  };
  glLineWidth(lineThickness);
  drawMeshImmediate(verts, 2, DRAW_LINES);
}

void Renderer::drawTexturedAABB2(const aabb2& bounds, 
								 const Texture& texture, 
								 const vec2& texCoordsAtMins, 
								 const vec2& texCoordsAtMaxs, 
								 const Rgba& tint) {
  bindTexutre(&texture);
  Vertex_PCU verts[6] = {
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

void Renderer::drawTextInBox2D(const aabb2& bounds, const std::string& asciiText, float cellHeight, vec2 aligns, TextDrawMode drawMode, const BitmapFont* font, const Rgba& tint, float aspectScale) {
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

  mCurrentTexture = createOrGetTexture("$");

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
  glDepthFunc(g_openGLCompare[compare]);

  // enable/disable write
  glDepthMask(shouldWrite ? GL_TRUE : GL_FALSE);
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

void Renderer::bindTexutre(const Texture* texture) {
  if (texture == nullptr) {
//    glDisable(GL_TEXTURE_2D);
//    UNIMPLEMENTED();
    return;
  }
//  glEnable(GL_TEXTURE_2D);
//  UNIMPLEMENTED();
  mCurrentTexture = texture;

  resetAlphaBlending();
//  UNIMPLEMENTED();
}

void Renderer::setTexture(const char* path) {
  bindTexutre(createOrGetTexture(path));
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

void Renderer::scale2D(float ratioX, float ratioY, float ratioZ) {
//	glScalef(ratioX, ratioY, ratioZ);
  UNIMPLEMENTED();
}

void Renderer::swapBuffers(HDC ctx) {
	SwapBuffers(ctx);
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
  if(filled) drawTexturedAABB2(bounds, *mTextures.at("$"), { 0,1 }, { 1,0 }, color);
  else {
    bindTexutre(mTextures.at("$"));  
    auto vertices = bounds.vertices();
    Vertex_PCU verts[6] = {
      { vec3(vertices[0]), color, vec2{ 0,0 } },
      { vec3(vertices[1]), color,vec2{ 0,0 } },
      { vec3(vertices[2]), color,vec2{ 0,0 } },
      { vec3(vertices[3]), color, vec2{ 0,0 } }
    };
    drawMeshImmediate(verts, 4, DRAW_LINE_LOOP);
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

  if(!filled) {
    drawMeshImmediate(verts.data(), verts.size(), DRAW_LINE_LOOP);
  } else {
    verts.emplace_back(vec2{ radius + center.x, center.y }, color, vec2::zero);
    drawMeshImmediate(verts.data(), verts.size(), DRAW_TRIANGLE_FAN);
  }
}

void Renderer::drawCube(const vec3& bottomCenter, const vec3& dimension, 
                        const Rgba& color, 
                        rect uvTop, rect uvSide, rect uvBottom) {
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
    std::array<Vertex_PCU, 6> mesh = {
      Vertex_PCU(vertices[0], color, uvs[0]),
      Vertex_PCU(vertices[1], color, uvs[1]),
      Vertex_PCU(vertices[2], color, uvs[2]),

      Vertex_PCU(vertices[0], color, uvs[0]),
      Vertex_PCU(vertices[2], color, uvs[2]),
      Vertex_PCU(vertices[3], color, uvs[3]),
    };

    drawMeshImmediate(mesh, DRAW_TRIANGES);
  }

  { // bottom
    auto uvs = uvBottom.vertices();
    std::array<Vertex_PCU, 6> mesh = {
      Vertex_PCU(vertices[4], color, uvs[0]),
      Vertex_PCU(vertices[5], color, uvs[1]),
      Vertex_PCU(vertices[6], color, uvs[2]),

      Vertex_PCU(vertices[4], color, uvs[0]),
      Vertex_PCU(vertices[6], color, uvs[2]),
      Vertex_PCU(vertices[7], color, uvs[3]),
    };

    drawMeshImmediate(mesh, DRAW_TRIANGES);
  }

  // four sides
    auto uvs = uvSide.vertices();
  {
    std::array<uint, 6> indices = { 4, 5, 1, 4, 1,0 };
    for(uint i = 0; i<3; i++) {
      std::array<Vertex_PCU, 6> mesh = {
        Vertex_PCU(vertices[(indices[0] + i)%8], color, uvs[0]),
        Vertex_PCU(vertices[(indices[1] + i)%8], color, uvs[1]),
        Vertex_PCU(vertices[(indices[2] + i)%8], color, uvs[2]),
        Vertex_PCU(vertices[(indices[3] + i)%8], color, uvs[0]),
        Vertex_PCU(vertices[(indices[4] + i)%8], color, uvs[2]),
        Vertex_PCU(vertices[(indices[5] + i)%8], color, uvs[3]),
      };

      drawMeshImmediate(mesh, DRAW_TRIANGES);
    }
  }

  {
    std::array<uint, 6> indices = { 7, 4, 0, 7, 0, 3 };
    std::array<Vertex_PCU, 6> mesh = {
      Vertex_PCU(vertices[indices[0]], color, uvs[0]),
      Vertex_PCU(vertices[indices[1]], color, uvs[1]),
      Vertex_PCU(vertices[indices[2]], color, uvs[2]),
      Vertex_PCU(vertices[indices[3]], color, uvs[0]),
      Vertex_PCU(vertices[indices[4]], color, uvs[2]),
      Vertex_PCU(vertices[indices[5]], color, uvs[3]),
    };

    drawMeshImmediate(mesh, DRAW_TRIANGES);
  }
}

void Renderer::drawMeshImmediate(const Vertex_PCU* vertices, size_t numVerts, DrawPrimitive drawPrimitive) {
  // first, copy the memory to the buffer
  mTempRenderBuffer.copyToGpu(sizeof(Vertex_PCU) * numVerts, vertices);
  
  //--------------------------bind position---------------------------------

  // Describe the buffer - first, figure out where the shader is expecting
  // position to be.
  GLint posBind = glGetAttribLocation(mCurrentShaderProgram->programHandle, "POSITION");

  // Next, bind the buffer we want to use; 
  glBindBuffer(GL_ARRAY_BUFFER, mTempRenderBuffer.handle);

  // next, bind where position is in our buffer to that location; 
  if (posBind >= 0) {
    // enable this location
    glEnableVertexAttribArray(posBind);

    // describe the data
    glVertexAttribPointer(posBind, // where?
                          3,                           // how many (vec3 has 3 floats)
                          GL_FLOAT,                    // type? (vec3 is 3 floats)
                          GL_FALSE,                    // Should data be normalized
                          sizeof(Vertex_PCU),              // stride (how far between each vertex)
                          (GLvoid*)offsetof(Vertex_PCU, position)); // From the start of a vertex, where is this data?
  }
  //--------------------------bind UV---------------------------------

  GLint uvBind = glGetAttribLocation(mCurrentShaderProgram->programHandle, "UV");
  glBindBuffer(GL_ARRAY_BUFFER, mTempRenderBuffer.handle);

  if (uvBind >= 0) {
    glEnableVertexAttribArray(uvBind);
    glVertexAttribPointer(uvBind,
                          2,                           
                          GL_FLOAT,                    
                          GL_FALSE,                   
                          sizeof(Vertex_PCU),
                          (GLvoid*)offsetof(Vertex_PCU, uvs));
  }

  //-------------------------Texture---------------------------------
  uint textureIndex = 0U; // to see how they tie together

                          // Bind the sampler;
  glBindSampler(textureIndex, mDefaultSampler->getHandle());

  // Bind the texture
  glActiveTexture(GL_TEXTURE0 + textureIndex);
  glBindTexture(GL_TEXTURE_2D, mCurrentTexture->getHandle());

  //-------------------------Color-----------------------------------
  // Next, bind the buffer we want to use;
  glBindBuffer(GL_ARRAY_BUFFER, mTempRenderBuffer.handle);

  // next, bind where position is in our buffer to that location;
  GLint bind = glGetAttribLocation(mCurrentShaderProgram->programHandle, "COLOR");
  if (bind >= 0) {
    // enable this location
    glEnableVertexAttribArray(bind);

    // describe the data
    glVertexAttribPointer(bind, // where?
                          4,                           // how many (RGBA is 4 unsigned chars)
                          GL_UNSIGNED_BYTE,            // type? (RGBA is 4 unsigned chars)
                          GL_TRUE,                     // Normalize components, maps 0-255 to 0-1.
                          sizeof(Vertex_PCU),              // stride (how far between each vertex)
                          (GLvoid*)offsetof(Vertex_PCU, color)); // From the start of a vertex, where is this data?
  }

  // Now that it is described and bound, draw using our program
  glUseProgram(mCurrentShaderProgram->programHandle);

  //-----------------------Matrix------------------------------------
  GLint loc = glGetUniformLocation(mCurrentShaderProgram->programHandle, "PROJECTION");
  if (loc >= 0) {
    // you "may" need to use GL_TRUE, depending on your matrix layout
    // and whether you prefer to multiply left or right;
    // acts on the currently bound program (glUseProgram)
    glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mCurrentCamera->mProjMatrix);
  }

  loc = glGetUniformLocation(mCurrentShaderProgram->programHandle, "VIEW");
  if (loc >= 0) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mCurrentCamera->mViewMatrix);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, mCurrentCamera->getFrameBufferHandle());
  glDrawArrays(g_openGlPrimitiveTypes[drawPrimitive], 0, numVerts);

}

void Renderer::cleanScreen(const Rgba& color) {
	float r = 0, g = 0 , b = 0, a = 1;
  color.getAsFloats(r, g, b, a);

  glClearColor(r,g,b,a);
	glClear(GL_COLOR_BUFFER_BIT);
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



#include <windows.h>

#include <map>
#include <string>

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "BitmapFont.hpp"
#include "SpriteSheet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <array>
#include "glFunctions.hpp"

#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library


int g_openGlPrimitiveTypes[NUM_PRIMITIVE_TYPES] =
{
  GL_POINTS,			// called PRIMITIVE_POINTS		in our engine
  GL_LINES,			// called PRIMITIVE_LINES		in our engine
  GL_LINE_LOOP,
  GL_TRIANGLES,		// called PRIMITIVE_TRIANGES	in our engine
  GL_TRIANGLE_FAN,
  GL_QUADS			// called PRIMITIVE_QUADS		in our engine
};

using Vertices = std::vector<Vertex_PCU>;

Renderer::Renderer() {
	loadIdentity();
  m_textures["$"] = new Texture();
}

Renderer::~Renderer() {
  for (const auto& kv : m_fonts) {
    delete kv.second;
  }

	for (const auto& kv: m_textures) {
		delete kv.second;
	}

  //QA: when to do the shutdown
  wglMakeCurrent(m_hdc, nullptr);

  ::wglDeleteContext(m_glContext);
  ::ReleaseDC(m_glWnd, m_hdc);

  m_glContext = nullptr;
  m_hdc = nullptr;
  m_glWnd = nullptr;

}

void Renderer::afterFrame() {
	SwapBuffers(m_hdc);
}

void Renderer::beforeFrame() {
//  glClearColor(1.f, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::drawLine(const Vector2& start, const Vector2& end, 
						const Rgba& startColor, const Rgba& endColor, float lineThickness) {
  bindTexutre(m_textures.at("$"));
  Vertex_PCU verts[2] = {
    { start, startColor, {0,0}},
    { end, endColor, {0,1}}
  };
  UNIMPLEMENTED();
//  glLineWidth(lineThickness);
  drawMeshImmediate(verts, 2, DRAW_LINES);
}

void Renderer::drawTexturedAABB2(const AABB2& bounds, 
								 const Texture& texture, 
								 const Vector2& texCoordsAtMins, 
								 const Vector2& texCoordsAtMaxs, 
								 const Rgba& tint) {
  bindTexutre(&texture);
  Vertex_PCU verts[4] = {
    { bounds.mins, tint, texCoordsAtMins },
    { Vector2{ bounds.maxs.x, bounds.mins.y }, tint, Vector2{ texCoordsAtMaxs.x, texCoordsAtMins.y } },
    { bounds.maxs, tint, texCoordsAtMaxs },
    { Vector2{ bounds.mins.x, bounds.maxs.y }, tint, Vector2{ texCoordsAtMins.x, texCoordsAtMaxs.y } },
  };

  drawMeshImmediate(verts, 4, DRAW_QUADS);
}

void Renderer::drawTexturedAABB2(const AABB2& bounds, const Texture& texture, 
                                 const AABB2& texCoords, const Rgba& tint) {
  drawTexturedAABB2(bounds, texture, texCoords.mins, texCoords.maxs, tint);
}

void Renderer::drawText2D(const Vector2& drawMins, const std::string& asciiText, 
                          float cellHeight, const Rgba& tint, 
                          float aspectScale, const BitmapFont* font) {
  GUARANTEE_OR_DIE(font != nullptr, "no font assigned");

  float charWidth = font->getStringWidth(asciiText, cellHeight, aspectScale) / asciiText.length();
  Vector2 dx(charWidth, 0.f), dy(0.f, cellHeight);

  for(int i =0; i<(int)asciiText.length(); i++) {
    Vector2 mins = drawMins + float(i) * dx;
    AABB2 textBounds(mins, mins + dx + dy);
    AABB2 uv = font->getUVsForGlyph(asciiText[i]);
    drawTexturedAABB2(textBounds, font->m_spriteSheet.getTexture(), uv, tint);
  }
}

void Renderer::drawText2D(const Vector2& drawMins, const std::string& asciiText, float cellHeight, const BitmapFont* font, const Rgba& tint, float aspectScale) {
  drawText2D(drawMins, asciiText, cellHeight, tint, aspectScale, font);
}

void Renderer::drawTextInBox2D(const AABB2& bounds, const std::string& asciiText, float cellHeight, Vector2 aligns, TextDrawMode drawMode, const BitmapFont* font, const Rgba& tint, float aspectScale) {
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

  Vector2 anchor(bounds.mins.x, bounds.maxs.y);

  Vector2 padding = bounds.getDimensions() - Vector2(blockWidth, blockHeight);
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
  m_glWnd = hwnd;
  m_hdc = hdc;
  m_glContext = real_context;

  postInit();

  return true;
}

void Renderer::postInit() {
  // default_vao is a GLuint member variable
  glGenVertexArrays(1, &m_defaultVao);
  glBindVertexArray(m_defaultVao);

  m_defaultShaderProgram = createOrGetShaderProgram("@default");
  m_currentShaderProgram = m_defaultShaderProgram;
}

void Renderer::setOrtho2D(const Vector2& bottomLeft, const Vector2& topRight) {
  loadIdentity();
//	glOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f);
  UNIMPLEMENTED();
  //glOrtho(0.f, SCREEN_WIDTH, 0.f, SCREEN_HEIGHT, 0.f, 1.f);
}

void Renderer::pushMatrix() {
//	glPushMatrix();
  UNIMPLEMENTED();
}

void Renderer::popMatrix() {
//	glPopMatrix();
  UNIMPLEMENTED();
}

void Renderer::traslate2D(const Vector2& translation) {
//	glTranslatef(translation.x, translation.y, 0);
  UNIMPLEMENTED();
}

void Renderer::useShaderProgram(ShaderProgram* program) {
  m_currentShaderProgram = program == nullptr ? m_shaderPrograms.at("@default"): program;
  ENSURES(m_currentShaderProgram != nullptr);
}

void Renderer::setAddtiveBlending() {
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  UNIMPLEMENTED();
}

void Renderer::resetAlphaBlending() {
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  UNIMPLEMENTED();
}

bool Renderer::reloadShaderProgram() {
  bool success = true;

  for(auto& kv: m_shaderPrograms) {
    success = success & kv.second->fromFile(kv.first.c_str());
  }

  return success;
}

bool Renderer::reloadShaderProgram(const char* nameWithPath) {
  std::string name = std::string(nameWithPath);
  auto it = m_shaderPrograms.find(name);

  EXPECTS(it != m_shaderPrograms.end());

  return it->second->fromFile(nameWithPath);
}

void Renderer::bindTexutre(const Texture* texture) {
  if (texture == nullptr) {
//    glDisable(GL_TEXTURE_2D);
    UNIMPLEMENTED();
    return;
  }
//  glEnable(GL_TEXTURE_2D);
  UNIMPLEMENTED();
  //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//  glBindTexture(GL_TEXTURE_2D, texture->m_textureID);
  UNIMPLEMENTED();
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

void Renderer::loadIdentity() {
//	glLoadIdentity();
  UNIMPLEMENTED();
}

void Renderer::drawAABB2(const AABB2& bounds, const Rgba& color, bool filled) {
  if(filled) drawTexturedAABB2(bounds, *m_textures.at("$"), { 0,1 }, { 1,0 }, color);
  else {
    bindTexutre(m_textures.at("$"));  
    auto vertices = bounds.vertices();
    Vertex_PCU verts[4] = {
      {
        Vector3(vertices[0]),
        color,
        Vector2{ 0,0 }
      },{
        Vector3(vertices[1]),
        color,
        Vector2{ 0,0 }
      },{
        Vector3(vertices[2]),
        color,
        Vector2{ 0,0 }
      },{
        Vector3(vertices[3]),
        color,
        Vector2{ 0,0 }
      } };
    drawMeshImmediate(verts, 4, DRAW_LINE_LOOP);
  }
}

void Renderer::drawCircle(const Vector2& center, float radius, const Rgba& color, bool filled) {
  Vertices verts;
  verts.reserve(21);

  if(filled) {
    verts.emplace_back(center, color, Vector2::zero);
  }

  for(int ii = 0; ii < 20; ii++) {
    float theta = 2.0f * PI * float(ii) * 0.05f;//get the current angle 

    float x = radius * cosf(theta);//calculate the x component 
    float y = radius * sinf(theta);//calculate the y component 

    verts.emplace_back(Vector2{ x + center.x, y + center.y }, color, Vector2::zero);
  }

  if(!filled) {
    drawMeshImmediate(verts.data(), verts.size(), DRAW_LINE_LOOP);
  } else {
    verts.emplace_back(Vector2{ radius + center.x, center.y }, color, Vector2::zero);
    drawMeshImmediate(verts.data(), verts.size(), DRAW_TRIANGLE_FAN);
  }
}

void Renderer::drawMeshImmediate(const Vertex_PCU* vertices, int numVerts, DrawPrimitive drawPrimitive) {
  // first, copy the memory to the buffer
  m_tempRenderBuffer.copyToGpu(sizeof(Vertex_PCU) * numVerts, vertices);

  // Describe the buffer - first, figure out where the shader is expecting
  // position to be.
  GLint posBind = glGetAttribLocation(m_currentShaderProgram->programHandle, "POSITION");

  // Next, bind the buffer we want to use; 
  glBindBuffer(GL_ARRAY_BUFFER, m_tempRenderBuffer.handle);

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

  // Now that it is described and bound, draw using our program
  glUseProgram(m_currentShaderProgram->programHandle);
  glDrawArrays(g_openGlPrimitiveTypes[drawPrimitive], 0, numVerts);

}

void Renderer::cleanScreen(const Rgba& color) {
	float r = 0, g = 0 , b = 0, a = 1;
	color.getAsFloats(r, g, b, a);
  UNIMPLEMENTED();

//  glClearColor(r,g,b,a);
//	glClear(GL_COLOR_BUFFER_BIT);// TODO: move to renderer
}

BitmapFont* Renderer::createOrGetBitmapFont(const char* bitmapFontName, const char* path) {
  const char* fullPath = std::string(path).append(bitmapFontName).c_str();
  return createOrGetBitmapFont(fullPath);
}

BitmapFont* Renderer::createOrGetBitmapFont(const char* fontNameWithPath) {
  auto kv = m_fonts.find(fontNameWithPath);
  if (kv != m_fonts.end()) {
    return kv->second;
  }

  Texture* fontTex = createOrGetTexture(fontNameWithPath);
  BitmapFont* font = new BitmapFont(fontNameWithPath, *(new SpriteSheet(*fontTex, 16, 16)));
  m_fonts[fontNameWithPath] = font;

  return font;
}

Texture* Renderer::createOrGetTexture(const std::string& filePath) {
	auto it = m_textures.find(filePath);
	if (it == m_textures.end()) {
		Texture* texture = new Texture(filePath);
		m_textures[filePath] = texture;
		return texture;
	}
	return it->second;
}

ShaderProgram* Renderer::createOrGetShaderProgram(const char* nameWithPath) {
  std::string name = std::string(nameWithPath);
  auto it = m_shaderPrograms.find(name);
  if (it != m_shaderPrograms.end()) return it->second;

  ShaderProgram* program = new ShaderProgram();
  bool success = program->fromFile(nameWithPath);

  if (!success) {
    program->fromFile("@invalid");
  };
  
  m_shaderPrograms[name] = program;
  return program;
}

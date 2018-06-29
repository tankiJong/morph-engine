#include "Draw.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/Gradient.hpp"
#include "Engine/Renderer/Geometry/Mesher.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Renderer/Shader/Material.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Math/Primitives/aabb3.hpp"

Renderer* gRenderer = nullptr;
Camera* gCamera = nullptr;
Camera* gCamera2D = nullptr;
const Clock* gDefaultColock = nullptr;

float gDefaultDuration = Debug::INF;
Debug::eDebugDrawDepthMode gDefaultDepthMode = Debug::DEBUG_DEPTH_DEFAULT;
Material* gDebugMaterial = nullptr;
Mesher gDebugMesher;

bool gEnabled = true;
struct Debug::DebugDrawMeta {
  Debug::DrawHandle handle;
  owner<Mesh*> mesh = nullptr;
  float duration;
  Debug::eDebugDrawDepthMode depthMode;
  Gradient decayColor;
  Camera* cam = nullptr;
  const Clock* clock = nullptr;
  mutable double endSec = 0;

  DebugDrawMeta(const Gradient& decayColor, Mesh* mesh, float duration, const Clock* clockOverride)
    : mesh(mesh)
    , duration(duration)
    , depthMode(gDefaultDepthMode)
    , decayColor(decayColor)
    , cam(gCamera)
    , clock(clockOverride == nullptr ? gDefaultColock : clockOverride) {
    handle.mTarget = this;
    handle.id = DrawHandle::next();
    endSec = duration == INF ? INF : clock->total.second + duration;
  }

  double progress(double current) const {
    
    return (duration == INF || duration == 0)? 0 : (current - endSec + duration) / duration;
  }

  void terminate() const {
    endSec = clock->total.second;
  }

  virtual void render() {
    vec4 col = this->decayColor.evaluate((float)this->progress(this->clock->total.second)).normalized();
    gDebugMaterial->setProperty("tint", col);

    gRenderer->setMaterial(gDebugMaterial, 0);
    gRenderer->setCamera(this->cam);

    switch(depthMode) { 
      case DEBUG_DEPTH_DEFAULT: break;
      case DEBUG_DEPTH_ENABLE:
        gRenderer->enableDepth(COMPARE_LESS, false);
      break;
      case DEBUG_DEPTH_DISABLE:
        gRenderer->enableDepth(COMPARE_ALWAYS, false);
      break;
      case DEBUG_DEPTH_XRAY:
        INFO("unsupported depth mode for debug draw");
      break;
    }

    gRenderer->drawMesh(*this->mesh);
  }

  virtual ~DebugDrawMeta() {
    delete mesh;
  }
};

struct DebugDrawMetaText: public Debug::DebugDrawMeta {
  DebugDrawMetaText(const Gradient& decayColor, const Font& font, Mesh* mesh, float duration, const Clock* clockOverride)
    : DebugDrawMeta(decayColor, mesh, duration, clockOverride), font(font) {}
  const Font& font;
  virtual void render() {
    vec4 col = this->decayColor.evaluate((float)this->progress(this->clock->total.second)).normalized();
    gDebugMaterial->setProperty("tint", col);

    g_theRenderer->setMaterial(gDebugMaterial, 1);

    g_theRenderer->setTexture(TEXTURE_DIFFUSE,font.texture(0));
    gRenderer->setSampler(0, &Sampler::Linear());
    gRenderer->setCamera(this->cam);

    switch (depthMode) {
      case Debug::DEBUG_DEPTH_DEFAULT: break;
      case Debug::DEBUG_DEPTH_ENABLE:
        gRenderer->enableDepth(COMPARE_LESS, false);
        break;
      case Debug::DEBUG_DEPTH_DISABLE:
        gRenderer->enableDepth(COMPARE_ALWAYS, false);
        break;
      case Debug::DEBUG_DEPTH_XRAY:
        INFO("unsupported depth mode for debug draw");
        break;
    }

    gRenderer->drawMesh(*this->mesh);
  }
};
std::vector<Debug::DebugDrawMeta*> gDebugDrawCalls;

bool Debug::DrawHandle::terminate() const {
  for(DebugDrawMeta* d: gDebugDrawCalls) {
    if (d == mTarget) {
      mTarget->terminate();
      return true;
    }
  }
  return false;
}

void Debug::setRenderer(Renderer* renderer) {
  EXPECTS(renderer != nullptr);

  gRenderer = renderer;
}

void Debug::setCamera2D(Camera* camera) {
  gCamera2D = camera;
}

void Debug::setCamera(Camera* camera) {
  gCamera = camera;
}

void Debug::setClock(const Clock* clock) {
  if(clock == nullptr) {
    gDefaultColock = &GetMainClock();
  } else {
    gDefaultColock = clock;
  }
}

void Debug::setDuration(float time) {
  EXPECTS(time >= 0);
  gDefaultDuration = time;
}

void Debug::setDepth(eDebugDrawDepthMode depthMode) {
  gDefaultDepthMode = depthMode;
}

void Debug::clear() {
  for(auto& dc: gDebugDrawCalls) {
    delete dc;
  }
  gDebugDrawCalls.clear();
}

void Debug::toggleDebugRender(bool isEnabled) {
  gEnabled = isEnabled;
}

void Debug::toggleDebugRender() {
  gEnabled = !gEnabled;
}

template<typename F>
Debug::DrawHandle* drawMeta(const Gradient& color, float duration, const Clock* clockOverride, F&& f, bool is3D = true) {
  static_assert(std::is_invocable_v<F, Mesher>);
  Mesher& mesher = gDebugMesher;
  mesher.clear();

  f(mesher);

  Debug::DebugDrawMeta* meta = new Debug::DebugDrawMeta(color, mesher.createMesh(), duration, clockOverride);
  if(!is3D) {
    meta->cam = gCamera2D;
  }
  gDebugDrawCalls.push_back(meta);
  meta->duration = duration;

  return reinterpret_cast<Debug::DrawHandle*>(meta);
}

template<typename F>
Debug::DrawHandle* drawMetaText(const Gradient& color, const Font* font, float duration, const Clock* clockOverride, F&& f, bool is3D = true) {
  static_assert(std::is_invocable_v<F, Mesher>);
  Mesher& mesher = gDebugMesher;
  mesher.clear();

  f(mesher);

  if(font == nullptr) {
    font = Font::Default().get();
  }

  Debug::DebugDrawMeta* meta = new DebugDrawMetaText(color, *font, mesher.createMesh<vertex_pcu_t>(), duration, clockOverride);
  if (!is3D) {
    meta->cam = gCamera2D;
  }
  gDebugDrawCalls.push_back(meta);
  meta->duration = duration;

  return reinterpret_cast<Debug::DrawHandle*>(meta);
}

void Debug::drawNow() {
  detail::Debug::tick();
  if(!gEnabled) return;
  // will change to material later
  if(!gDebugMaterial) {
    gDebugMaterial = Resource<Material>::clone("material/debug/default");
  }
  for(uint i = 0; i < gDebugDrawCalls.size(); ++i) {
    DebugDrawMeta*& comp = gDebugDrawCalls[i];
    
    comp->render();
    if(comp->endSec <= comp->clock->total.second) {
      delete comp;
      comp = gDebugDrawCalls.back();
      gDebugDrawCalls.pop_back();
      i--;
      continue;
    }
  }
}

const Debug::DrawHandle* Debug::drawQuad2(const vec2& a, const vec2& b, const vec2& c, const vec2& d, float duration, const Gradient& cl,
  const Clock* clockOverride) {
  
  return drawMeta(cl, duration, clockOverride, [=](Mesher& mesher) {

    mesher.begin(DRAW_TRIANGES);
    mesher.color(Rgba::white);
    uint start = 
    mesher.vertex2f(a);
    mesher.vertex2f(b);
    mesher.vertex2f(c);
    mesher.vertex2f(d);

    mesher.triangle(start, start + 1, start + 2)
          .triangle(start, start + 2, start + 3);
    mesher.end();
  }, false);

}

const Debug::DrawHandle* Debug::drawLine2(const vec2& a, const vec2& b, float duration, const Rgba& cla, const Rgba& clb,
  const Clock* clockOverride) {
  
  return drawMeta(Rgba::white, duration, clockOverride, [&](Mesher& mesher) {
    mesher.begin(DRAW_LINES, false);

    mesher.color(cla);
    mesher.vertex2f(a);
    mesher.color(clb);
    mesher.vertex2f(b);

    mesher.end();
  }, false);
}

const Debug::DrawHandle* Debug::drawText2(std::string_view text, float size, const vec2& bottomLeft, float duration,
  const Gradient& cl, const Font* font) {
  return drawMetaText(cl, font, duration, nullptr, [&](Mesher& mesher) {

    mesher.begin(DRAW_TRIANGES)
      .color(Rgba::white)
      .text(text, size,
            font == nullptr ? Font::Default().get() : font, 
            vec3(bottomLeft, 0), vec3::right, vec3::up)
      .end();
  }, false);
}

const Debug::DrawHandle* Debug::drawPoint(const vec3& position, float duration, const Gradient& color, const Clock* clockOverride) {

  return drawMeta(color, duration, clockOverride, [&](Mesher& mesher) {
    mesher.begin(DRAW_LINES, false);

    mesher.color(Rgba::white);
    mesher.vertex3f(vec3::right + position);
    mesher.vertex3f(-vec3::right + position);
    mesher.vertex3f(vec3::up + position);
    mesher.vertex3f(-vec3::up + position);
    mesher.vertex3f(vec3::forward + position);
    mesher.vertex3f(-vec3::forward + position);

    mesher.end();
  });

}

const Debug::DrawHandle* Debug::drawPoint(const vec3& position, const Gradient& color, float duration, const Clock* clockOverride) {
  return drawPoint(position, duration, color, clockOverride);
}

const Debug::DrawHandle* Debug::drawLine(const vec3& from, const vec3& to, float /*thickness*/, float duration, const Rgba& colorStart,
  const Rgba& colorEnd, const Clock* clockOverride) {

  TODO("introduce thickness when use mesh to draw line")
  return drawMeta(Rgba::white, duration, clockOverride, [&](Mesher& mesher) {
    mesher.begin(DRAW_LINES, false);

    mesher.color(colorStart);
    mesher.vertex3f(from);
    mesher.color(colorEnd);
    mesher.vertex3f(to);

    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawTri(const vec3& a, const vec3& b, const vec3& c, float duration, const Gradient& cl,
  const Clock* clockOverride) {
  return drawMeta(cl, duration, clockOverride, [&](Mesher& mesher) {
    mesher.color(Rgba::white);

    mesher.begin(DRAW_TRIANGES, false);

    mesher.vertex3f(a);
    mesher.vertex3f(b);
    mesher.vertex3f(c);

    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawQuad(const vec3& a, const vec3& b, const vec3& c, const vec3& d, float duration,
  const Gradient& cl, const Clock* clockOverride) {

  return drawMeta(cl, duration, clockOverride, [&](Mesher& mesher) {
    mesher.color(Rgba::white);

    mesher.begin(DRAW_TRIANGES);

    uint start =
    mesher.vertex3f(a);
    mesher.vertex3f(b);
    mesher.vertex3f(c);
    mesher.vertex3f(d);

    mesher.triangle(start, start + 1, start + 2)
      .triangle(start, start + 2, start + 3);
    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawCone(const vec3& origin,
  const vec3& direction,
  float length,
  float angle,
  uint slides,
  float duration,
  bool framed,
  const Gradient& color,
  const Clock* clockoverride) {
  return drawMeta(color, duration, clockoverride, [&](Mesher& mesher) {
    mesher.color(Rgba::white);

    mesher.begin(framed ? DRAW_LINES: DRAW_TRIANGES)
      .cone(origin, direction, length, angle, slides)
      .end();
  });
}

const Debug::DrawHandle* Debug::drawSphere(const vec3& center, float size, uint levelX, uint levelY, float duration, const Gradient& color,
  const Clock* clockoverride) {
  return drawMeta(color, duration, clockoverride, [&](Mesher& mesher) {
    mesher.color(Rgba::white);
    mesher
      .begin(DRAW_LINES)
      .sphere(center, size, levelX, levelY)
      .end();

  });
}

const Debug::DrawHandle* Debug::drawCube(const vec3& center, const vec3& dimension, bool framed, float duration, const Gradient& cl,
  const Clock* clockOverride) {

  return drawMeta(cl, duration, clockOverride, [&](Mesher& mesher) {
    mesher.color(Rgba::white);
    vec3 bottomCenter = center - vec3::up * dimension.y * .5f;
    float dx = dimension.x * .5f, dy = dimension.y * .5f, dz = dimension.z * .5f;

    std::array<vec3, 8> vertices = {
      bottomCenter + vec3{ -dx, 2.f * dy, -dz },
      bottomCenter + vec3{ dx, 2.f * dy, -dz },
      bottomCenter + vec3{ dx, 2.f * dy,  dz },
      bottomCenter + vec3{ -dx, 2.f * dy,  dz },

      bottomCenter + vec3{ -dx, 0, -dz },
      bottomCenter + vec3{ dx, 0, -dz },
      bottomCenter + vec3{ dx, 0,  dz },
      bottomCenter + vec3{ -dx, 0,  dz }
    };

    mesher.begin(framed ? DRAW_LINES : DRAW_TRIANGES);
    
    mesher.vertex3f(vertices);

    mesher.quad(0, 1, 2, 3)
          .quad(4, 5, 6, 7)
          .quad(4, 5, 1, 0)
          .quad(5, 6, 2, 1)
          .quad(6, 7, 3, 2)
          .quad(7, 4, 0, 3);

    mesher.end();
  });
  
}

const Debug::DrawHandle* Debug::drawCube(const vec3& center, float size, bool framed, float duration, const Gradient& cl,
  const Clock* clockOverride) {
  return drawCube(center, vec3(size), framed, duration, cl, clockOverride);
}

const Debug::DrawHandle* Debug::drawCube(const aabb3& box,
  const mat44& localToWorld,
  bool framed,
  float duration,
  const Gradient& cl,
  const Clock* clockOverride) {

  vec3 corners[8];
  box.corners(corners);

  for(vec3& corner: corners) {
    corner = (localToWorld * vec4(corner, 1.f)).xyz();
  }

  return drawMeta(cl, duration, clockOverride, [&](Mesher& mesher) {
    mesher.begin(framed ? DRAW_LINES : DRAW_TRIANGES);
    mesher.quad(corners[3], corners[2], corners[1], corners[0]);
    mesher.quad(corners[4], corners[5], corners[6], corners[7]);
    mesher.quad(corners[0], corners[1], corners[5], corners[4]);
    mesher.quad(corners[1], corners[2], corners[6], corners[5]);
    mesher.quad(corners[2], corners[3], corners[7], corners[6]);
    mesher.quad(corners[3], corners[0], corners[4], corners[7]);
    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawBasis(const vec3& position, const vec3& i, const vec3& j, const vec3& k,
  float duration, Clock* clockOverride) {
  Debug::DrawHandle* handle =  drawMeta(Rgba::white, duration, clockOverride, [&](Mesher& mesher) {
    mesher.begin(DRAW_LINES, false);

    mesher.color(Rgba::red);
    mesher.vertex3f(position + vec3::zero);
    mesher.vertex3f(position + i);

    mesher.color(Rgba::green);
    mesher.vertex3f(position + vec3::zero);
    mesher.vertex3f(position + j);

    mesher.color(Rgba::blue);
    mesher.vertex3f(position + vec3::zero);
    mesher.vertex3f(position + k);

    mesher.end();
  });

  DebugDrawMeta* meta = reinterpret_cast<DebugDrawMeta*>(handle);

  meta->decayColor = Gradient();
  return handle;
}

const Debug::DrawHandle* Debug::drawBasis(const Transform& transform, float duration, Clock* clockOverride) {
  return drawBasis(transform.position(), transform.right(), transform.up(), transform.forward(), duration, clockOverride);
}

const Debug::DrawHandle* Debug::drawGrid(const vec3& center, const vec3& right, const vec3 forward, float unitSize,
  float limit, float duration, const Gradient& cl, const Clock* clockOverride) {

  return drawMeta(cl, duration, clockOverride, [&](Mesher& mesher) {
    vec3 xStart = center - right * limit, xEnd = center + right * limit;
    vec3 yStart = center - forward * limit, yEnd = center + forward * limit;

    float i = 0.f;
    
    mesher.begin(DRAW_LINES, false)
          .color(Rgba::white);
    while(i * unitSize < limit) {
      float step = i * unitSize;

      mesher
        .line(xStart + forward * step, xEnd + forward * step)
        .line(yStart + right * step, yEnd + right * step)

        .line(xStart - forward * step, xEnd - forward * step)
        .line(yStart - right * step, yEnd - right * step);

      i++;
    }
    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawText(std::string_view text, float size, const vec3& bottomLeft, float duration,
  const vec3& direction, const vec3& up, const Font* font, const Gradient& cl, const Clock* clockOverride) {
  return drawMetaText(cl, font, duration, clockOverride, [&](Mesher& mesher) {

    mesher.begin(DRAW_TRIANGES)
      .color(Rgba::white)
      .text(text, size, font == nullptr ? Font::Default().get() : font, bottomLeft, direction, up)
      .end();

  });
}


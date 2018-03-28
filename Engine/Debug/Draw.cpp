#include "Draw.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/Gradient.hpp"
#include "Engine/Renderer/Geometry/Mesher.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/glFunctions.hpp"
#include "Engine/Math/Curves.hpp"

Renderer* gRenderer = nullptr;
Camera* gCamera = nullptr;
const Clock* gDefaultColock = nullptr;

float gDefaultDuration = Debug::INF;
Debug::eDebugDrawDepthMode gDefaultDepthMode = Debug::DEBUG_DEPTH_DEFAULT;
Gradient gDecayColor;
ShaderProgram* gShaderProg = nullptr;
Mesher gDebugMesher;

struct Debug::DebugDrawMeta {
  Debug::DrawHandle handle;
  Mesh* mesh = nullptr;
  float duration;
  Debug::eDebugDrawDepthMode depthMode;
  Gradient decayColor;
  Camera* cam = nullptr;
  const Clock* clock = nullptr;
  float endSec = 0;
  DebugDrawMeta(Mesh* mesh, float duration, const Clock* clockOverride)
    : mesh(mesh), duration(duration)
    , depthMode(gDefaultDepthMode), decayColor(gDecayColor)
    , cam(gCamera), clock(clockOverride == nullptr ? gDefaultColock : clockOverride) {
    handle.id = DrawHandle::next();
    endSec = duration == INF ? INF : clock->total.second + duration;
  }

  float progress(float current) const {
    
    return duration == INF ? 0 : (current - endSec + duration) / duration;
  }

  void terminate() {
    endSec = clock->total.second;
  }
};

std::vector<Debug::DebugDrawMeta*> gDebugDrawCalls;

void Debug::setRenderer(Renderer* renderer) {
  EXPECTS(renderer != nullptr);

  gRenderer = renderer;
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

void Debug::setDecayColor(const Rgba& from, const Rgba& to) {
  gDecayColor.clear();
  gDecayColor.add(from, 0.f).add(to, 1.f);
}

template<typename F>
Debug::DrawHandle* drawMeta(float duration, const Clock* clockOverride, F&& f) {
  static_assert(std::is_invocable_v<F, Mesher>);
  Mesher& mesher = gDebugMesher;
  mesher.clear();

  f(mesher);

  Debug::DebugDrawMeta* meta = new Debug::DebugDrawMeta(mesher.createMesh(), duration, clockOverride);
  gDebugDrawCalls.push_back(meta);

  meta->duration = duration;

  ENSURES((void*)meta == &(meta->handle));
  return reinterpret_cast<Debug::DrawHandle*>(meta);
}

void Debug::drawNow() {
  static UniformBuffer* tintUniform = UniformBuffer::For(vec4::zero);
  // will change to material later
  if(!gShaderProg) {
    gShaderProg = g_theRenderer->createOrGetShaderProgram("Data/debug");
  }
  g_theRenderer->useShaderProgram(gShaderProg);
  for(uint i = 0; i < gDebugDrawCalls.size(); ++i) {
    DebugDrawMeta*& comp = gDebugDrawCalls[i];

    if(comp->endSec < comp->clock->total.second) {
      delete comp;
      comp = gDebugDrawCalls.back();
      gDebugDrawCalls.pop_back();
      i--;
      continue;
    }
    Rgba col = comp->decayColor.evaluate(comp->progress(comp->clock->total.second));
    vec4 colf;
    col.getAsFloats(colf.x, colf.y, colf.z, colf.w);

    tintUniform->set(colf);

    g_theRenderer->setUniform(UNiFORM_USER_1, *tintUniform);
    gRenderer->setCamera(comp->cam);
    gRenderer->drawMesh(*comp->mesh);
    GL_CHECK_ERROR();

  }
}

const Debug::DrawHandle* Debug::drawQuad2(const vec2& a, const vec2& b, const vec2& c, const vec2& d, float duration, const Rgba& cl,
  const Clock* clockOverride) {
  
  return drawMeta(duration, clockOverride, [=](Mesher& mesher) {
    mesher.color(cl);

    mesher.begin(DRAW_TRIANGES);

    uint start = 
    mesher.vertex2f(a);
    mesher.vertex2f(b);
    mesher.vertex2f(c);
    mesher.vertex2f(d);

    mesher.triangle(start, start + 1, start + 2)
          .triangle(start, start + 2, start + 3);
    mesher.end();
  });

}

const Debug::DrawHandle* Debug::drawLine2(const vec2& a, const vec2& b, float duration, const Rgba& cla, const Rgba& clb,
  const Clock* clockOverride) {
  
  return drawMeta(duration, clockOverride, [&](Mesher& mesher) {
    mesher.begin(DRAW_LINES, false);

    mesher.color(cla);
    mesher.vertex2f(a);
    mesher.color(clb);
    mesher.vertex2f(b);

    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawPoint(const vec3& position, float duration, const Rgba& color, const Clock* clockOverride) {

  return drawMeta(duration, clockOverride, [&](Mesher& mesher) {
    mesher.begin(DRAW_LINES, false);

    mesher.color(color);
    mesher.vertex3f(vec3::right);
    mesher.vertex3f(-vec3::right);
    mesher.vertex3f(vec3::up);
    mesher.vertex3f(-vec3::up);
    mesher.vertex3f(vec3::forward);
    mesher.vertex3f(-vec3::forward);

    mesher.end();
  });

}

const Debug::DrawHandle* Debug::drawPoint(const vec3& position, const Rgba& color, float duration, const Clock* clockOverride) {
  return drawPoint(position, gDefaultDuration, color, clockOverride);
}

const Debug::DrawHandle* Debug::drawLine(const vec3& from, const vec3& to, float thickness, float duration, const Rgba& colorStart,
  const Rgba& colorEnd, const Clock* clockOverride) {
  return drawMeta(duration, clockOverride, [&](Mesher& mesher) {
    mesher.begin(DRAW_LINES, false);

    mesher.color(colorStart);
    mesher.vertex3f(from);
    mesher.color(colorEnd);
    mesher.vertex3f(to);

    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawTri(const vec3& a, const vec3& b, const vec3& c, float duration, const Rgba& cl,
  const Clock* clockOverride) {
  return drawMeta(duration, clockOverride, [&](Mesher& mesher) {
    mesher.color(cl);

    mesher.begin(DRAW_TRIANGES, false);

    mesher.vertex3f(a);
    mesher.vertex3f(b);
    mesher.vertex3f(c);

    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawQuad(const vec3& a, const vec3& b, const vec3& c, const vec3& d, float duration,
  const Rgba& cl, const Clock* clockOverride) {

  return drawMeta(duration, clockOverride, [&](Mesher& mesher) {
    mesher.color(cl);

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

const Debug::DrawHandle* Debug::drawSphere(const vec3& center, float size, uint levelX, uint levelY, float duration, const Rgba& color,
  const Clock* clockoverride) {
  return drawMeta(duration, clockoverride, [&](Mesher& mesher) {
    mesher.color(color);
    mesher
      .begin(DRAW_LINES)
      .sphere(center, size, levelX, levelY)
      .end();

  });
}

const Debug::DrawHandle* Debug::drawCube(const vec3& center, const vec3& dimension, float duration, const Rgba& cl,
  const Clock* clockOverride) {

  return drawMeta(duration, clockOverride, [&](Mesher& mesher) {
    mesher.color(cl);
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

    mesher.begin(DRAW_TRIANGES);
    
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

const Debug::DrawHandle* Debug::drawCube(const vec3& center, float size, float duration, const Rgba& cl,
  const Clock* clockOverride) {
  return drawCube(center, vec3(size), duration, cl, clockOverride);
}

const Debug::DrawHandle* Debug::drawBasis(const vec3& position, const vec3& i, const vec3& j, const vec3& k,
  float duration, Clock* clockOverride) {
  Debug::DrawHandle* handle =  drawMeta(duration, clockOverride, [&](Mesher& mesher) {
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


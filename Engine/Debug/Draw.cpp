#include "Draw.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/Gradient.hpp"
#include "Engine/Graphics/Font.hpp"
#include "Engine/Graphics/Model/Mesher.hpp"
#include "Engine/Math/Primitives/aabb3.hpp"
#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/ImmediateRenderer.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Debug/Debug_Wireframe_ps.h"
#include "Debug/Debug_Wireframe_vs.h"
#include "Debug/Debug_Text_ps.h"
#include "Debug/Debug_Text_vs.h"
#include "Engine/Graphics/Camera.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Graphics/RHI/PipelineState.hpp"
ImmediateRenderer* gRenderer = nullptr;

float gDefaultDuration = Debug::INF;
Camera* gDefaultCamera = nullptr;
U<Camera> gUICamera = nullptr;
const Clock* gDefaultClock = nullptr;
Debug::eDebugDrawDepthMode gDefaultDepthMode = Debug::DEBUG_DEPTH_ENABLE;

Mesher gDebugMesher;
S<const Program> gDebugProgram;
S<const Program> gDebugProgramDepthAlways;
// RHIBuffer::sptr_t gTintBuffer;

bool gEnabled = true;
bool gEnableGpuRecord = false;


TypedBuffer::sptr_t gGpuDebugMesh;
TypedBuffer::sptr_t gFakeGpuDebugMesh;

TypedBuffer::sptr_t& getGpuMesh() {
  if(gEnableGpuRecord) {
    return gGpuDebugMesh;
  } else {
    return gFakeGpuDebugMesh;
  }
}
class Debug::DebugDrawMetaData {
public:
  virtual ~DebugDrawMetaData() = default;

  Debug::DrawHandle handle;
  mutable double endSec = 0;
  DrawOption options;
  DebugDrawMetaData(DrawOption options)
    : options(options) {
    handle.mTarget = this;
    handle.id = DrawHandle::next();
    endSec = options.duration == INF ? INF : (options.clock->total.second + options.duration);
  }

  float progress() const {
    double current = options.clock->total.second;
    return (float)((options.duration == INF || options.duration == 0)? 0 : (current - endSec + options.duration) / options.duration);
  }

  void terminate() const {
    endSec = options.clock->total.second;
  }

  virtual void appendSubMesh(Mesher& mesher) = 0;
  virtual void prepareDraw(ImmediateRenderer& renderer) = 0;
  // virtual void render() {
  //   vec4 col = this->options.decayColor.evaluate((float)this->progress(this->options.clock->total.second)).normalized();
  //
  //   gRenderer->setView(*this->options.camera);
  //
  //   switch(options.depthMode) { 
  //     case DEBUG_DEPTH_ENABLE:
  //       gRenderer->setProgram(gDebugProgram);
  //     break;
  //     case DEBUG_DEPTH_DISABLE:
  //       gRenderer->setProgram(gDebugProgramDepthAlways);
  //     break;
  //     case DEBUG_DEPTH_XRAY:
  //       INFO("unsupported depth mode for debug draw");
  //     break;
  //   }
  //   
  // gRenderer->drawMesh(*this->mesh);
  // }
};

template<typename MesherCreator>
class DebugDrawMetaShape: public Debug::DebugDrawMetaData {
public:
  static_assert(std::is_invocable_v<MesherCreator, Mesher&, DebugDrawMetaData&>);
  MesherCreator meshCreator;

  DebugDrawMetaShape(MesherCreator&& creator, Debug::DrawOption options)
    : DebugDrawMetaData(options)
    , meshCreator(std::forward<MesherCreator>(creator)) {
    
  }

  void appendSubMesh(Mesher& mesher) override {
    meshCreator(mesher, *this);
  };

  void prepareDraw(ImmediateRenderer& renderer) override {

    switch(options.depthMode) { 
      case Debug::DEBUG_DEPTH_ENABLE: {
        auto prog = Resource<Program>::get("internal/Shader/debug/default");
        renderer.setProgram(prog);
      }
      break;
      case Debug::DEBUG_DEPTH_DISABLE: {
        auto prog = Resource<Program>::get("internal/Shader/debug/always");
        renderer.setProgram(prog);
      }
      break;
      case Debug::DEBUG_DEPTH_XRAY: 
        BAD_CODE_PATH();
      break;
    }

  }
};

template<typename MesherCreator>
class DebugDrawMetaText: public Debug::DebugDrawMetaData {
public:
  MesherCreator meshCreator;
  const  Font* font;
  DebugDrawMetaText(MesherCreator&& creator, const Font* font, Debug::DrawOption options)
    : DebugDrawMetaData(options)
    , meshCreator(std::forward<MesherCreator>(creator))                   
    , font(font) {}

  void appendSubMesh(Mesher& mesher) override {
    meshCreator(mesher, *this);
  };

  void prepareDraw(ImmediateRenderer& renderer) override {

    switch(options.depthMode) { 
      case Debug::DEBUG_DEPTH_ENABLE: {
        auto prog = Resource<Program>::get("internal/Shader/debug/text_default");
        renderer.setProgram(prog);
      }
      break;
      case Debug::DEBUG_DEPTH_DISABLE: {
        auto prog = Resource<Program>::get("internal/Shader/debug/text_always");
        renderer.setProgram(prog);
      }
      break;
      case Debug::DEBUG_DEPTH_XRAY: 
        BAD_CODE_PATH();
      break;
    }

    renderer.setTexture(TEXTURE_DIFFUSE, *font->texture(0)->srv());

  };

};


std::vector<Debug::DebugDrawMetaData*> gDebugDrawCalls;

bool Debug::DrawHandle::terminate() const {
  for(DebugDrawMetaData* d: gDebugDrawCalls) {
    if (d == mTarget) {
      mTarget->terminate();
      return true;
    }
  }
  return false;
}

Debug::DrawOption::DrawOption()
: depthMode(gDefaultDepthMode)
, clock(gDefaultClock)
, duration(0)
, camera(gDefaultCamera) 
, decayColor(Rgba::white){ }

void Debug::setRenderer(ImmediateRenderer* renderer) {
  EXPECTS(renderer != nullptr);

  gRenderer = renderer;
}

void Debug::setCamera(Camera* camera) {
  gDefaultCamera = camera;
}

void Debug::setClock(const Clock* clock) {
  if(clock == nullptr) {
    gDefaultClock = &GetMainClock();
  } else {
    gDefaultClock = clock;
  }
}

void Debug::setDuration(float time) {
  EXPECTS(time >= 0);
  gDefaultDuration = time;
}

void Debug::setDepth(eDebugDrawDepthMode depthMode) {
  gDefaultDepthMode = depthMode;
}

void Debug::enableGpuRecord() {
  gEnableGpuRecord = true;
  resetUavMesh();
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

void Debug::drawInit() {
  gRenderer = &ImmediateRenderer::get();
  gUICamera.reset(new Camera());
  aabb2 size = Window::Get()->bounds();
  gUICamera->setProjectionOrtho(size.width(), size.height(), -100, 100);

  gGpuDebugMesh = 
    TypedBuffer::create(sizeof(vertex_pcu_t), 4000000, 
                        RHIResource::BindingFlag::UnorderedAccess | RHIResource::BindingFlag::ShaderResource);
  NAME_RHIRES(gGpuDebugMesh);
  gFakeGpuDebugMesh =
    TypedBuffer::create(sizeof(vertex_pcu_t), 4000000, 
                        RHIResource::BindingFlag::UnorderedAccess | RHIResource::BindingFlag::ShaderResource);
  NAME_RHIRES(gFakeGpuDebugMesh);

}

template<typename F>
Debug::DrawHandle* drawMetaShape(Debug::DrawOption options, F&& f) {
  static_assert(std::is_invocable_v<F, Mesher&, Debug::DebugDrawMetaData&>);

  // Mesher& mesher = gDebugMesher;
  // mesher.clear();
  // mesher.color(Rgba::white);
  // f(mesher);

  Debug::DebugDrawMetaData* meta = new DebugDrawMetaShape(std::forward<F>(f), options);

  gDebugDrawCalls.push_back(meta);
  // meta->duration = duration;

  return reinterpret_cast<Debug::DrawHandle*>(meta);
}

template<typename F>
Debug::DrawHandle* drawMetaText(const Debug::DrawOption& options, const Font* font, F&& f) {
  static_assert(std::is_invocable_v<F, Mesher&, Debug::DebugDrawMetaData&>);

  // f(mesher);

  if(font == nullptr) {
    font = Font::Default().get();
  }

  Debug::DebugDrawMetaData* meta = new DebugDrawMetaText(std::forward<F>(f), font, options);

  gDebugDrawCalls.push_back(meta);

  return reinterpret_cast<Debug::DrawHandle*>(meta);
}

DeclVertexType(v_debug) {
  vec3 position{ 0.f };
  vec4 color{ 1.f, 1.f, 1.f, 1.f };
  vec2 uvs{ 0.f };
  v_debug() = default;
};

DefineVertexType(v_debug) {
  define(0, "POSITION", MP_FLOAT, 3, 0, true, 0);
  define(0, "COLOR",    MP_FLOAT, 4, 3, false, 0);
  define(0, "UV",       MP_FLOAT, 2, 7, true, 0);
}
void debugDrawGpuMesh() {
  auto defaultProg = Resource<Program>::get("internal/Shader/debug/default");
  auto alwaysProg = Resource<Program>::get("internal/Shader/debug/always");

  static GraphicsState::sptr_t defaultpps;
  static GraphicsState::sptr_t alwayspps;

  auto sig = gRenderer->defaultRootSignature();
  GraphicsState::PrimitiveType prim = GraphicsState::PrimitiveType::Line;
  if(defaultpps == nullptr) {
  
    GraphicsState::Desc desc;
    gRenderer->context()->bindDescriptorHeap();

    desc.setRootSignature(sig);

    desc.setFboDesc(gRenderer->defaultFrameBuffer()->desc());
    desc.setVertexLayout(VertexLayout::For<v_debug>());
    desc.setProgram(defaultProg);
    desc.setRootSignature(sig);

    desc.setPrimTye(prim);

    defaultpps = GraphicsState::create(desc);
  }
  if(alwayspps == nullptr) {
    sig = gRenderer->defaultRootSignature();
  
    GraphicsState::Desc desc;
    gRenderer->context()->bindDescriptorHeap();

    desc.setRootSignature(sig);

    desc.setFboDesc(gRenderer->defaultFrameBuffer()->desc());
    desc.setVertexLayout(VertexLayout::For<v_debug>());
    desc.setProgram(alwaysProg);
    desc.setRootSignature(sig);


    desc.setPrimTye(prim);

    alwayspps = GraphicsState::create(desc);
  }

  S<RHIContext> ctx = gRenderer->context();
 
  ctx->setGraphicsState(*defaultpps);
  gRenderer->defaultDescriptorSet()->bindForGraphics(*ctx, *sig);
  ctx->setFrameBuffer(*gRenderer->defaultFrameBuffer());

  ctx->setVertexBuffer(*gGpuDebugMesh, 0);
  ctx->setPrimitiveTopology(DRAW_LINES);

  struct DrawArgs {
    UINT VertexCountPerInstance;
    UINT InstanceCount = 1;
    UINT StartVertexLocation = 0;
    UINT StartInstanceLocation = 0;
  };

  DrawArgs drawArgs;
  static RHIBuffer::sptr_t buffer = RHIBuffer::create(sizeof(DrawArgs), RHIResource::BindingFlag::IndirectArg, RHIBuffer::CPUAccess::None, &drawArgs);

  ctx->copyBufferRegion(buffer.get(), 0, gGpuDebugMesh->uavCounter().get(), 0, sizeof(UINT));
  ctx->drawIndirect(*buffer, 1, 0);
}

void Debug::drawNow() {
  SCOPED_GPU_EVENT(*RHIDevice::get()->defaultRenderContext(), "Debug draw");
  if(!gEnabled) return;

  gRenderer->setRenderTarget(RHIDevice::get()->backBuffer()->rtv(), 0);
  gRenderer->setDepthStencilTarget(RHIDevice::get()->depthBuffer()->dsv());

  gRenderer->setRenderRegion(*RHIDevice::get()->backBuffer());

  gDebugMesher.clear();

  for(uint i = 0; i < gDebugDrawCalls.size(); ++i) {
    DebugDrawMetaData*& comp = gDebugDrawCalls[i];
    comp->appendSubMesh(gDebugMesher);
  }

  Mesh* immediateMesh = gDebugMesher.createMesh<vertex_pcu_t>();

  for(uint i = 0; i < gDebugDrawCalls.size(); i++) {
    DebugDrawMetaData*& comp = gDebugDrawCalls[i];

    gRenderer->setView(*comp->options.camera);

    comp->prepareDraw(*gRenderer);
    gRenderer->drawSubMesh(*immediateMesh, i);

  }

  for(uint i = 0; i < gDebugDrawCalls.size(); i++) {
    DebugDrawMetaData*& comp = gDebugDrawCalls[i];

    if(comp->endSec <= comp->options.clock->total.second) {
      delete comp;
      comp = gDebugDrawCalls.back();
      gDebugDrawCalls.pop_back();
      i--;
      continue;
    }
  }

  SAFE_DELETE(immediateMesh);

  debugDrawGpuMesh();
  gEnableGpuRecord = false;
}

const Debug::DrawHandle* Debug::drawQuad2(const vec2& a, const vec2& b, const vec2& c, const vec2& d, float duration, const Gradient& cl,
  const Clock* clockOverride) {

  DrawOption options;
  options.duration = duration;
  options.decayColor = cl;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  options.camera = gUICamera.get();
  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& meta) {

    mesher.begin(DRAW_TRIANGES);
    mesher.color(meta.options.decayColor.evaluate(meta.progress()));
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
  
  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  options.camera = gUICamera.get();

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& /*meta*/) {
    mesher.begin(DRAW_LINES, false);

    mesher.color(cla);
    mesher.vertex2f(a);
    mesher.color(clb);
    mesher.vertex2f(b);

    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawText2(std::string text, float size, const vec2& bottomLeft, float duration,
  const Gradient& cl, const Font* font) {

  DrawOption options;
  options.duration = duration;
  options.decayColor = cl;
  options.camera = gUICamera.get();

  return drawMetaText(options, font, [=](Mesher& mesher, DebugDrawMetaData& meta) {

    mesher.begin(DRAW_TRIANGES)
      .color(meta.options.decayColor.evaluate(meta.progress()))
      .text(text, size,
            font == nullptr ? Font::Default().get() : font, 
            vec3(bottomLeft, 0), vec3::right, vec3::up)
      .end();
  });
}

const Debug::DrawHandle* Debug::drawPoint(const vec3& position, float size, float duration, const Gradient& color, const Clock* clockOverride) {

  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  options.decayColor = color;

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& meta) {
    mesher.begin(DRAW_LINES, false);

    static const vec3 right = (vec3::right + vec3::up + vec3::forward) * 0.33333f;
    static const vec3 up = (-vec3::right + vec3::up + vec3::forward) * 0.33333f;
    static const vec3 fwd = (-vec3::right - vec3::up + vec3::forward) * 0.33333f;

    mesher.color(meta.options.decayColor.evaluate(meta.progress()));
    mesher.vertex3f(right * size * .5f + position);
    mesher.vertex3f(-right * size * .5f + position);
    mesher.vertex3f(up * size * .5f + position);
    mesher.vertex3f(-up * size * .5f + position);
    mesher.vertex3f(fwd * size * .5f + position);
    mesher.vertex3f(-fwd * size * .5f + position);

    mesher.end();
  });

}

const Debug::DrawHandle* Debug::drawPoint(const vec3& position, const Gradient& color, float duration, const Clock* clockOverride) {
  return drawPoint(position, 1.f, duration, color, clockOverride);
}

const Debug::DrawHandle* Debug::drawLine(const vec3& from, const vec3& to, float /*thickness*/, float duration, const Rgba& colorStart,
  const Rgba& colorEnd, const Clock* clockOverride) {

  TODO("introduce thickness when use mesh to draw line")

  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData&) {
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

  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  options.decayColor = cl;

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& meta) {
    mesher.begin(DRAW_TRIANGES, false);

    mesher.color(meta.options.decayColor.evaluate(meta.progress()));

    mesher.vertex3f(a);
    mesher.vertex3f(b);
    mesher.vertex3f(c);

    mesher.end();
  });
}

const Debug::DrawHandle* Debug::drawQuad(const vec3& a, const vec3& b, const vec3& c, const vec3& d, float duration,
  const Gradient& cl, const Clock* clockOverride) {

  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  options.decayColor = cl;

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& meta) {
    mesher.begin(DRAW_TRIANGES);

    mesher.color(meta.options.decayColor.evaluate(meta.progress()));

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

  DrawOption options;
  options.duration = duration;
  options.clock = clockoverride == nullptr ? gDefaultClock : clockoverride;
  options.decayColor = color;

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& meta) {
    mesher.color(meta.options.decayColor.evaluate(meta.progress()));

    mesher.begin(framed ? DRAW_LINES: DRAW_TRIANGES)
      .cone(origin, direction, length, angle, slides)
      .end();

  });
}

const Debug::DrawHandle* Debug::drawSphere(const vec3& center, float size, uint levelX, uint levelY, bool framed, 
                                           float duration, const Gradient& color, const Clock* clockoverride) {
  
  DrawOption options;
  options.duration = duration;
  options.clock = clockoverride == nullptr ? gDefaultClock : clockoverride;
  options.decayColor = color;

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& meta) {
    mesher.color(meta.options.decayColor.evaluate(meta.progress()));
    mesher
      .begin(framed ? DRAW_LINES : DRAW_TRIANGES)
      .sphere(center, size, levelX, levelY)
      .end();

  });
}

const Debug::DrawHandle* Debug::drawCube(const vec3& center, const vec3& dimension, bool framed, float duration, const Gradient& cl,
  const Clock* clockOverride) {

  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  options.decayColor = cl;

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& meta) {
    mesher.color(meta.options.decayColor.evaluate(meta.progress()));
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
    
    uint start = mesher.vertex3f(vertices);

    mesher.quad(start + 0, start + 1, start + 2, start + 3)
      .quad(start + 7, start + 6, start + 5, start + 4)
      .quad(start + 4, start + 5, start + 1, start + 0)
      .quad(start + 5, start + 6, start + 2, start + 1)
      .quad(start + 6, start + 7, start + 3, start + 2)
      .quad(start + 7, start + 4, start + 0, start + 3);


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

  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  options.decayColor = cl;

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& meta) {
    mesher.begin(framed ? DRAW_LINES : DRAW_TRIANGES);
    mesher.color(meta.options.decayColor.evaluate(meta.progress()));
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

  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  
  Debug::DrawHandle* handle =  drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData&) {
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

  return handle;
}

const Debug::DrawHandle* Debug::drawBasis(const Transform& transform, float duration, Clock* clockOverride) {
  return drawBasis(transform.position(), transform.right(), transform.up(), transform.forward(), duration, clockOverride);
}

const Debug::DrawHandle* Debug::drawGrid(const vec3& center, const vec3& right, const vec3 forward, float unitSize,
  float limit, float duration, const Gradient& cl, const Clock* clockOverride) {

  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  options.decayColor = cl;

  return drawMetaShape(options, [=](Mesher& mesher, DebugDrawMetaData& meta) {
    vec3 xStart = center - right * limit, xEnd = center + right * limit;
    vec3 yStart = center - forward * limit, yEnd = center + forward * limit;

    float i = 0.f;
    
    mesher.begin(DRAW_LINES, false)
          .color(meta.options.decayColor.evaluate(meta.progress()));
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

const Debug::DrawHandle* Debug::drawText(std::string text, float size, const vec3& bottomLeft, float duration,
  const vec3& direction, const vec3& up, const Font* font, const Gradient& cl, const Clock* clockOverride) {
  
  DrawOption options;
  options.duration = duration;
  options.clock = clockOverride == nullptr ? gDefaultClock : clockOverride;
  options.decayColor = cl;

  return drawMetaText(options, font, [=](Mesher& mesher, DebugDrawMetaData& meta) {

    mesher.begin(DRAW_TRIANGES)
      .color(meta.options.decayColor.evaluate(meta.progress()))
      .text(text, size, font == nullptr ? Font::Default().get() : font, bottomLeft, direction, up)
      .end();

  });
}

TypedBuffer::sptr_t Debug::uavMesh() {
  return getGpuMesh();
}

void Debug::resetUavMesh() {
  u32 zero = 0;
  getGpuMesh()->uavCounter()->updateData(zero);
}

DEF_RESOURCE(Program, "internal/Shader/debug/default") {
  Program::sptr_t prog = Program::sptr_t(new Program());

  prog->stage(SHADER_TYPE_VERTEX).setFromBinary(gDebug_Wireframe_vs, sizeof(gDebug_Wireframe_vs));
  prog->stage(SHADER_TYPE_FRAGMENT).setFromBinary(gDebug_Wireframe_ps, sizeof(gDebug_Wireframe_ps));
  prog->compile();

  prog->compile();
  RenderState state;

  state.isWriteDepth = FLAG_FALSE;
  state.depthMode = COMPARE_LESS;
  state.colorBlendOp = BLEND_OP_ADD;
  state.colorSrcFactor = BLEND_F_SRC_ALPHA;
  state.colorDstFactor = BLEND_F_DST_ALPHA;
  state.alphaBlendOp = BLEND_OP_ADD;
  state.alphaSrcFactor = BLEND_F_SRC_ALPHA;
  state.alphaDstFactor = BLEND_F_DST_ALPHA;
  state.cullMode = CULL_NONE;
  prog->setRenderState(state);

  return S<Program>(prog);
}

DEF_RESOURCE(Program, "internal/Shader/debug/always") {
  Program::sptr_t prog = Program::sptr_t(new Program());

  prog->stage(SHADER_TYPE_VERTEX).setFromBinary(gDebug_Wireframe_vs, sizeof(gDebug_Wireframe_vs));
  prog->stage(SHADER_TYPE_FRAGMENT).setFromBinary(gDebug_Wireframe_ps, sizeof(gDebug_Wireframe_ps));
  prog->compile();

  prog->compile();
  RenderState state;
  state.isWriteDepth = FLAG_FALSE;
  state.depthMode = COMPARE_ALWAYS;
  state.colorBlendOp = BLEND_OP_ADD;
  state.colorSrcFactor = BLEND_F_SRC_ALPHA;
  state.colorDstFactor = BLEND_F_DST_ALPHA;
  state.alphaBlendOp = BLEND_OP_ADD;
  state.alphaSrcFactor = BLEND_F_SRC_ALPHA;
  state.alphaDstFactor = BLEND_F_DST_ALPHA;
  state.cullMode = CULL_NONE;
  prog->setRenderState(state);

  return S<Program>(prog);
}


DEF_RESOURCE(Program, "internal/Shader/debug/text_default") {
  Program::sptr_t prog = Program::sptr_t(new Program());

  prog->stage(SHADER_TYPE_VERTEX).setFromBinary(gDebug_Text_vs, sizeof(gDebug_Text_vs));
  prog->stage(SHADER_TYPE_FRAGMENT).setFromBinary(gDebug_Text_ps, sizeof(gDebug_Text_ps));
  prog->compile();

  prog->compile();
  RenderState state;

  state.isWriteDepth = FLAG_FALSE;
  state.depthMode = COMPARE_LESS;
  state.colorBlendOp = BLEND_OP_ADD;
  state.colorSrcFactor = BLEND_F_SRC_ALPHA;
  state.colorDstFactor = BLEND_F_DST_ALPHA;
  state.alphaBlendOp = BLEND_OP_ADD;
  state.alphaSrcFactor = BLEND_F_SRC_ALPHA;
  state.alphaDstFactor = BLEND_F_DST_ALPHA;
  state.cullMode = CULL_NONE;
  prog->setRenderState(state);

  return S<Program>(prog);
}

DEF_RESOURCE(Program, "internal/Shader/debug/text_always") {
  Program::sptr_t prog = Program::sptr_t(new Program());

  prog->stage(SHADER_TYPE_VERTEX).setFromBinary(gDebug_Text_vs, sizeof(gDebug_Text_vs));
  prog->stage(SHADER_TYPE_FRAGMENT).setFromBinary(gDebug_Text_ps, sizeof(gDebug_Text_ps));
  prog->compile();

  prog->compile();
  RenderState state;
  state.isWriteDepth = FLAG_FALSE;
  state.depthMode = COMPARE_ALWAYS;
  state.colorBlendOp = BLEND_OP_ADD;
  state.colorSrcFactor = BLEND_F_SRC_ALPHA;
  state.colorDstFactor = BLEND_F_DST_ALPHA;
  state.alphaBlendOp = BLEND_OP_ADD;
  state.alphaSrcFactor = BLEND_F_SRC_ALPHA;
  state.alphaDstFactor = BLEND_F_DST_ALPHA;
  state.cullMode = CULL_NONE;
  prog->setRenderState(state);

  return S<Program>(prog);
}
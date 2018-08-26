#include "SceneRenderer.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/Camera.hpp"
#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Graphics/RHI/PipelineState.hpp"
#include "Engine/Graphics/RHI/VertexLayout.hpp"
#include "Engine/Graphics/Model/Vertex.hpp"
#include "Engine/Renderer/Renderable/Renderable.hpp"

#include "GenGBuffer_ps.h"
#include "GenGBuffer_vs.h"
#include "Engine/Graphics/Program/Material.hpp"
#include "Engine/Graphics/Model/Mesh.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

static Program::sptr_t gGenGBufferProgram = nullptr;
static RootSignature::scptr_t gGenBufferRootSig = nullptr;
SceneRenderer::SceneRenderer(const RenderScene & target): mTargetScene(target) {

}

void SceneRenderer::onLoad(RHIContext& ctx) {
  if(gGenGBufferProgram == nullptr) {
    gGenGBufferProgram = Program::sptr_t(new Program());
    gGenGBufferProgram->stage(SHADER_TYPE_VERTEX)
      .setFromBinary(gGenGBuffer_vs, sizeof(gGenGBuffer_vs));
    gGenGBufferProgram->stage(SHADER_TYPE_FRAGMENT)
      .setFromBinary(gGenGBuffer_ps, sizeof(gGenGBuffer_ps));
    gGenBufferRootSig = gGenGBufferProgram->rootSignature();
  }
  mFrameData.frameCount = 0;
  mFrameData.time = 0;

  auto size = Window::Get()->bounds().size();
  uint width = (uint)size.x;
  uint height = (uint)size.y;

  mGAlbedo = Texture2::create(width, height, TEXTURE_FORMAT_RGBA8,
                              RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource);
  mGNormal = Texture2::create(width, height, TEXTURE_FORMAT_RGBA8,
                              RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource);
  mGDepth = Texture2::create(width, height, TEXTURE_FORMAT_RGBA8,
                             RHIResource::BindingFlag::DepthStencil | RHIResource::BindingFlag::ShaderResource);
  {
    FrameBuffer::Desc desc;
    desc.defineColorTarget(0, TEXTURE_FORMAT_RGBA8, false); // albedo
    desc.defineColorTarget(1, TEXTURE_FORMAT_RGBA8, false); // normal
    desc.defineDepthTarget(TEXTURE_FORMAT_D24S8, false); // depth

    mGFbo.setDesc(desc);

    mGFbo.setColorTarget(&mGAlbedo->rtv(), 0);
    mGFbo.setColorTarget(&mGNormal->rtv(), 0);
    mGFbo.setDepthStencilTarget(mGDepth->dsv());
  }
}

void SceneRenderer::onRenderFrame(RHIContext& ctx) {
  setupFrame();
  setupView(ctx);
  genGBuffer(ctx);
}

void SceneRenderer::genGBuffer(RHIContext& ctx) {
  ctx.setFrameBuffer(mGFbo);

  GraphicsState::Desc desc;
  desc.setProgram(gGenGBufferProgram);
  desc.setFboDesc(mGFbo.desc());
  desc.setPrimTye(GraphicsState::PrimitiveType::Triangle);
  desc.setRootSignature(gGenBufferRootSig);
  desc.setVertexLayout(VertexLayout::For<vertex_lit_t>());

  GraphicsState::sptr_t gs = GraphicsState::create(desc);
  ctx.setGraphicsState(*gs);

  for(Renderable* r: mTargetScene.Renderables()) {
    r->material()->bindForGraphics(ctx, *gGenBufferRootSig, 0);
    r->mesh()->bindForContext(ctx);
    mcModel->updateData(r->transform()->localToWorld());

    for(const draw_instr_t& inst: r->mesh()->instructions()) {
      if(inst.useIndices) {
        ctx.drawIndexed(0, inst.startIndex, inst.elementCount);
      } else {
        ctx.draw(inst.startIndex, inst.elementCount);
      }
    }
  }
}

void SceneRenderer::setupFrame() {
  mFrameData.frameCount++;
  mFrameData.time = GetMainClock().total.second;
  mcFrameData->updateData(mFrameData);

  auto ctx = RHIDevice::get()->defaultRenderContext();
  ctx->clearRenderTarget(mGAlbedo->rtv(), Rgba::black);
  ctx->clearRenderTarget(mGNormal->rtv(), Rgba::gray);
  ctx->clearRenderTarget(mAO->rtv(), Rgba::white);
  ctx->clearDepthStencilTarget(*mGDepth->dsv(), true, true);
}

void SceneRenderer::setupView(RHIContext& ctx) {
  Camera& cam = *mTargetScene.camera();
  mcCamera->updateData(cam);
  ctx.setViewport({ vec2::zero, {(float)cam.width(), (float)cam.height()} });

}

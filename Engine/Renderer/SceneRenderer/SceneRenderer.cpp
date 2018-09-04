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
#include "GenAccelerationStructure_cs.h"
#include "GenAO_cs.h"
#include "Engine/Graphics/Program/Material.hpp"
#include "Engine/Graphics/Model/Mesh.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

static Program::sptr_t gGenGBufferProgram = nullptr;
static RootSignature::scptr_t gGenBufferRootSig = nullptr;

static Program::sptr_t gGenAOProgram = nullptr;
static RootSignature::scptr_t gGenAORootSig = nullptr;

static Program::sptr_t gGenAccelerationStructureProgram = nullptr;

struct surfel_t {
  vec3 position;
  vec3 normal;
};


SceneRenderer::SceneRenderer(const RenderScene & target): mTargetScene(target) {

}

void SceneRenderer::onLoad(RHIContext& ctx) {
  if(gGenGBufferProgram == nullptr) {
    gGenGBufferProgram = Program::sptr_t(new Program());
    gGenGBufferProgram->stage(SHADER_TYPE_VERTEX)
      .setFromBinary(gGenGBuffer_vs, sizeof(gGenGBuffer_vs));
    gGenGBufferProgram->stage(SHADER_TYPE_FRAGMENT)
      .setFromBinary(gGenGBuffer_ps, sizeof(gGenGBuffer_ps));
    gGenGBufferProgram->compile();
    gGenBufferRootSig = gGenGBufferProgram->rootSignature();

    gGenAOProgram = Program::sptr_t(new Program());
    gGenAOProgram->stage(SHADER_TYPE_COMPUTE)
      .setFromBinary(gGenAO_cs, sizeof(gGenAO_cs));
    gGenAOProgram->compile();
    gGenAORootSig = gGenAOProgram->rootSignature();

    gGenAccelerationStructureProgram = Program::sptr_t(new Program());
    gGenAccelerationStructureProgram->stage(SHADER_TYPE_COMPUTE)
      .setFromBinary(gGenAccelerationStructure_cs, sizeof(gGenAccelerationStructure_cs));
  }
  mFrameData.frameCount = 0;
  mFrameData.time = 0;

  auto size = Window::Get()->bounds().size();
  uint width = (uint)size.x;
  uint height = (uint)size.y;

  mAO = Texture2::create(width, height, TEXTURE_FORMAT_RGBA8,
                              RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::UnorderedAccess);
  NAME_RHIRES(mAO);
  mGAlbedo = Texture2::create(width, height, TEXTURE_FORMAT_RGBA8,
                              RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource);
  NAME_RHIRES(mGAlbedo);
  mGNormal = Texture2::create(width, height, TEXTURE_FORMAT_RGBA8,
                              RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource);
  NAME_RHIRES(mGNormal);
  mGPosition = Texture2::create(width, height, TEXTURE_FORMAT_RGBA16,
                              RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource);

  NAME_RHIRES(mGPosition);
  mGDepth = Texture2::create(width, height, TEXTURE_FORMAT_D24S8,
                             RHIResource::BindingFlag::DepthStencil | RHIResource::BindingFlag::ShaderResource);

  NAME_RHIRES(mGDepth);
  mcFrameData = RHIBuffer::create(sizeof(frame_data_t), RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);
  NAME_RHIRES(mcFrameData);
  mcCamera = RHIBuffer::create(sizeof(camera_t), RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);
  NAME_RHIRES(mcCamera);
  mcModel = RHIBuffer::create(sizeof(mat44), RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);
  NAME_RHIRES(mcModel);

  mSurfels = TypedBuffer::create(sizeof(surfel_t), 2048, RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::UnorderedAccess);
  NAME_RHIRES(mSurfels);

  {
    FrameBuffer::Desc desc;
    desc.defineColorTarget(0, TEXTURE_FORMAT_RGBA8, false); // albedo
    desc.defineColorTarget(1, TEXTURE_FORMAT_RGBA8, false); // normal
    desc.defineColorTarget(2, TEXTURE_FORMAT_RGBA16, false); // position
    desc.defineDepthTarget(TEXTURE_FORMAT_D24S8, false); // depth

    mGFbo.setDesc(desc);

    mGFbo.setColorTarget(&mGAlbedo->rtv(), 0);
    mGFbo.setColorTarget(&mGNormal->rtv(), 1);
    mGFbo.setColorTarget(&mGPosition->rtv(), 2);
    mGFbo.setDepthStencilTarget(mGDepth->dsv());
  }
  {
    DescriptorSet::Layout layout;
    layout.addRange(DescriptorSet::Type::Cbv, 0, 4);
    layout.addRange(DescriptorSet::Type::TextureSrv, 0, 1);

    mDSharedDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDSharedDescriptors->setCbv(0, 0, *mcFrameData->cbv());
    mDSharedDescriptors->setCbv(0, 1, *mcCamera->cbv());
    mDSharedDescriptors->setCbv(0, 2, *mcModel->cbv());
    mDSharedDescriptors->setCbv(0, 3, *ConstantBufferView::nullView());
    mDSharedDescriptors->setSrv(1, 0, *ShaderResourceView::nullView());
  }
  {
    DescriptorSet::Layout layout;
    layout.addRange(DescriptorSet::Type::TextureSrv, 10, 5);

    mDGBufferDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDGBufferDescriptors->setSrv(0, 0, mGAlbedo->srv());
    mDGBufferDescriptors->setSrv(0, 1, mGNormal->srv());
    mDGBufferDescriptors->setSrv(0, 2, mGPosition->srv());
    mDGBufferDescriptors->setSrv(0, 3, mGDepth->srv());
  }
  {
    DescriptorSet::Layout layout;
    layout.addRange(DescriptorSet::Type::TextureUav, 0, 1);

    mDGenAOUavDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDGenAOUavDescriptors->setUav(0, 0, *mAO->uav());
  }
}

void SceneRenderer::onRenderFrame(RHIContext& ctx) {
  ctx.beforeFrame();
  setupFrame();
  setupView(ctx);
  genGBuffer(ctx);
  genAO(ctx);
  ctx.copyResource(*mAO, *RHIDevice::get()->backBuffer());
}

void SceneRenderer::genGBuffer(RHIContext& ctx) {
  SCOPED_GPU_EVENT("Gen G-Buffer");
  ctx.setFrameBuffer(mGFbo);
  ctx.bindDescriptorHeap();
  GraphicsState::Desc desc;
  desc.setProgram(gGenGBufferProgram);
  desc.setFboDesc(mGFbo.desc());
  desc.setPrimTye(GraphicsState::PrimitiveType::Triangle);
  desc.setRootSignature(gGenBufferRootSig);
  desc.setVertexLayout(VertexLayout::For<vertex_lit_t>());

  GraphicsState::sptr_t gs = GraphicsState::create(desc);
  ctx.setGraphicsState(*gs);

  ctx.resourceBarrier(mGAlbedo.get(), RHIResource::State::RenderTarget);
  ctx.resourceBarrier(mGNormal.get(), RHIResource::State::RenderTarget);
  ctx.resourceBarrier(mGPosition.get(), RHIResource::State::RenderTarget);
  ctx.resourceBarrier(mGDepth.get(), RHIResource::State::DepthStencil);

  mDSharedDescriptors->bindForGraphics(ctx, *gGenBufferRootSig, 0);

  uint totalCount = 0;
  for(Renderable* r: mTargetScene.Renderables()) {
    r->material()->bindForGraphics(ctx, *gGenBufferRootSig, 1);
    r->mesh()->bindForContext(ctx);
    mcModel->updateData(r->transform()->localToWorld());

    for(const draw_instr_t& inst: r->mesh()->instructions()) {
      totalCount += inst.elementCount;
      if(inst.useIndices) {
        ctx.drawIndexed(0, inst.startIndex, inst.elementCount);
      } else {
        ctx.draw(inst.startIndex, inst.elementCount);
      }
    }
  }

  if (!mAccelerationStructure) {
    SCOPED_GPU_EVENT("Gen AccelerationStructure");
    DescriptorSet::Layout layout;

    mAccelerationStructure = TypedBuffer::create(sizeof(vec4), totalCount, RHIResource::BindingFlag::UnorderedAccess);
    NAME_RHIRES(mAccelerationStructure);

    // mapping with the vertex layout
    layout.addRange(DescriptorPool::Type::TypedBufferSrv, 0, 5);
    layout.addRange(DescriptorPool::Type::StructuredBufferUav, 0, 1);

    DescriptorSet::sptr_t descriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    RootSignature::Desc desc;
    desc.addDescriptorSet(layout);
    static RootSignature::sptr_t sig = RootSignature::create(desc);

    ComputeState::Desc pipdesc;

    pipdesc.setRootSignature(sig);
    pipdesc.setProgram(gGenAccelerationStructureProgram);

    static ComputeState::sptr_t computeState = ComputeState::create(pipdesc);

    ctx.setComputeState(*computeState);
    descriptors->bindForCompute(ctx, *sig, 0);

    descriptors->setUav(1, 0, *mAccelerationStructure->uav());
    for(uint i = 0; i < mTargetScene.Renderables().size(); i++) {
      Renderable* r = mTargetScene.Renderables()[i];
      for(auto& attribute: r->mesh()->layout().attributes()) {
        ctx.resourceBarrier(&r->mesh()->vertices(attribute.streamIndex)->res(), RHIResource::State::NonPixelShader);
        descriptors->setSrv(0, attribute.streamIndex, r->mesh()->vertices(attribute.streamIndex)->srv());
      }
      //
      // uint x = r->mesh()->vertices(0)->size() / 16 + 1;
      // uint y = 1;
      ctx.dispatch(1, 1, 1);
    }

    mDGBufferDescriptors->setSrv(0, 4, mAccelerationStructure->srv());

  }
}

void SceneRenderer::genAO(RHIContext& ctx) {
  SCOPED_GPU_EVENT("Gen AO");

  static ComputeState::sptr_t computeState;
  if(!computeState) {
    ComputeState::Desc desc;
    desc.setRootSignature(gGenAORootSig);
    desc.setProgram(gGenAOProgram);
    computeState = ComputeState::create(desc);
  }
  ctx.resourceBarrier(mGAlbedo.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGNormal.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGPosition.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGDepth.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mAccelerationStructure.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mAO.get(), RHIResource::State::UnorderedAccess);

  ctx.setComputeState(*computeState);

  // mDGenAOUavDescriptors->setUav(0, 1, mTargetScene.accelerationStructure());
  mDSharedDescriptors->bindForCompute(ctx, *gGenBufferRootSig, 0);
  mDGBufferDescriptors->bindForCompute(ctx, *gGenBufferRootSig, 1);
  mDGenAOUavDescriptors->bindForCompute(ctx, *gGenAORootSig, 2);

  uint x = uint(Window::Get()->bounds().width()) / 32 + 1;
  uint y = uint(Window::Get()->bounds().height()) / 32 + 1;

  ctx.dispatch(x, y, 1);
}

void SceneRenderer::accumlateSurfels(RHIContext& ctx) {
  
}

void SceneRenderer::setupFrame() {

  mFrameData.frameCount++;
  mFrameData.time = (float)GetMainClock().total.second;
  mcFrameData->updateData(mFrameData);

  auto ctx = RHIDevice::get()->defaultRenderContext();
  ctx->clearRenderTarget(mGAlbedo->rtv(), Rgba::black);
  ctx->clearRenderTarget(mGNormal->rtv(), Rgba::gray);
  ctx->clearRenderTarget(mAO->rtv(), Rgba::white);
  ctx->clearDepthStencilTarget(*mGDepth->dsv(), true, true);
}

void SceneRenderer::setupView(RHIContext& ctx) {
  Camera& cam = *mTargetScene.camera();
  mcCamera->updateData(cam.ubo());

  // TODO: setup viewport later
  ctx.setViewport({ vec2::zero, { Window::Get()->bounds().width(), Window::Get()->bounds().height()} });

}

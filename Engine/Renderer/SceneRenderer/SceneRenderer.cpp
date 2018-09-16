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
#include "Engine/Graphics/Program/Material.hpp"
#include "Engine/Graphics/Model/Mesh.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

#include "GenGBuffer_ps.h"
#include "GenGBuffer_vs.h"
#include "SurfelPlacement_cs.h"
#include "SurfelVisual_cs.h"
#include "SurfelGI_cs.h"
#include "GenAccelerationStructure_cs.h"
#include "GenAO_cs.h"
#include "DeferredLighting_cs.h"
#include "SurfelCoverageCompute_cs.h"
#include "Engine/Framework/Light.hpp"
#include "Engine/Input/Input.hpp"

static Program::sptr_t gGenGBufferProgram = nullptr;
static RootSignature::scptr_t gGenBufferRootSig = nullptr;

static Program::sptr_t gGenAOProgram = nullptr;
static RootSignature::scptr_t gGenAORootSig = nullptr;

static Program::sptr_t gGenAccelerationStructureProgram = nullptr;

static Program::sptr_t gSurfelPlacementProgram = nullptr;
static RootSignature::scptr_t gSurfelPlacementRootSig = nullptr;

static Program::sptr_t gSurfelVisualProgram = nullptr;
static RootSignature::scptr_t gSurfelVisualRootSig = nullptr;

static Program::sptr_t gSurfelCoverageProgram = nullptr;

static Program::sptr_t gDeferredLighting = nullptr;

static Program::sptr_t gSurfelGIProgram = nullptr;

struct surfel_t {
  vec3 position;
  vec3 normal;
  vec3 color;
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
    gGenAccelerationStructureProgram->compile();

    gSurfelPlacementProgram = Program::sptr_t(new Program());
    gSurfelPlacementProgram->stage(SHADER_TYPE_COMPUTE)
      .setFromBinary(gSurfelPlacement_cs, sizeof(gSurfelPlacement_cs));
    gSurfelPlacementProgram->compile();
    gSurfelPlacementRootSig = gSurfelPlacementProgram->rootSignature();

    gSurfelVisualProgram = Program::sptr_t(new Program());
    gSurfelVisualProgram->stage(SHADER_TYPE_COMPUTE)
      .setFromBinary(gSurfelVisual_cs, sizeof(gSurfelVisual_cs));
    gSurfelVisualProgram->compile();
    gSurfelVisualRootSig = gSurfelVisualProgram->rootSignature();

    gSurfelCoverageProgram = Program::sptr_t(new Program());
    gSurfelCoverageProgram->stage(SHADER_TYPE_COMPUTE)
      .setFromBinary(gSurfelCoverageCompute_cs, sizeof(gSurfelCoverageCompute_cs));
    gSurfelCoverageProgram->compile();

    gDeferredLighting = Program::sptr_t(new Program());
    gDeferredLighting->stage(SHADER_TYPE_COMPUTE)
      .setFromBinary(gDeferredLighting_cs, sizeof(gDeferredLighting_cs));
    gDeferredLighting->compile();

    gSurfelGIProgram = Program::sptr_t(new Program());
    gSurfelGIProgram->stage(SHADER_TYPE_COMPUTE)
      .setFromBinary(gSurfelGI_cs, sizeof(gSurfelGI_cs));
    gSurfelGIProgram->compile();
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

  mSurfels = TypedBuffer::create(sizeof(surfel_t), 20480, RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::UnorderedAccess);
  NAME_RHIRES(mSurfels);

  mcLight = RHIBuffer::create(sizeof(mat44), RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);
  NAME_RHIRES(mcLight);

  mSurfelVisual = Texture2::create(width, height, TEXTURE_FORMAT_RGBA8,
                              RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::UnorderedAccess);
  NAME_RHIRES(mSurfelVisual);

  mSurfelSpawnChance = Texture2::create(width, height, TEXTURE_FORMAT_RGBA8,
                                   RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::UnorderedAccess);
  NAME_RHIRES(mSurfelSpawnChance);

  mSurfelCoverage = Texture2::create(width, height, TEXTURE_FORMAT_RGBA16,
                                           RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::UnorderedAccess);
  NAME_RHIRES(mSurfelCoverage);

  mScene = Texture2::create(width, height, TEXTURE_FORMAT_RGBA8,
                            RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::UnorderedAccess);
  NAME_RHIRES(mScene);
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

    mDSharedDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDSharedDescriptors->setCbv(0, 0, *mcFrameData->cbv());
    mDSharedDescriptors->setCbv(0, 1, *mcCamera->cbv());
    mDSharedDescriptors->setCbv(0, 2, *mcModel->cbv());
    mDSharedDescriptors->setCbv(0, 3, *mcLight->cbv());
  }
  {
    DescriptorSet::Layout layout;
    layout.addRange(DescriptorSet::Type::TextureSrv, 10, 6);

    mDGBufferDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDGBufferDescriptors->setSrv(0, 0, mGAlbedo->srv());
    mDGBufferDescriptors->setSrv(0, 1, mGNormal->srv());
    mDGBufferDescriptors->setSrv(0, 2, mGPosition->srv());
    mDGBufferDescriptors->setSrv(0, 3, mGDepth->srv());
    mDGBufferDescriptors->setSrv(0, 4, *ShaderResourceView::nullView());
    mDGBufferDescriptors->setSrv(0, 5, *ShaderResourceView::nullView());
  }
  {
    DescriptorSet::Layout layout;
    layout.addRange(DescriptorSet::Type::TextureUav, 0, 1);

    mDGenAOUavDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDGenAOUavDescriptors->setUav(0, 0, *mAO->uav());
  }
  {
    DescriptorSet::Layout layout;
    layout.addRange(DescriptorSet::Type::StructuredBufferUav, 0, 4);
    layout.addRange(DescriptorSet::Type::TextureSrv, 0, 1);

    mDAccumulateSurfelUavDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDAccumulateSurfelUavDescriptors->setUav(0, 0, *mSurfels->uav());
    mDAccumulateSurfelUavDescriptors->setUav(0, 1, *mSurfels->uavCounter().uav());
    mDAccumulateSurfelUavDescriptors->setUav(0, 2, *mSurfelCoverage->uav());
    mDAccumulateSurfelUavDescriptors->setUav(0, 3, *mSurfelSpawnChance->uav());
    mDAccumulateSurfelUavDescriptors->setSrv(1, 0, mSurfelCoverage->srv());

  }
  {
    DescriptorSet::Layout layout;
    layout.addRange(DescriptorSet::Type::StructuredBufferUav, 0, 2);
    layout.addRange(DescriptorSet::Type::TextureUav, 0, 1);

    mDSurfelVisualDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDSurfelVisualDescriptors->setUav(0, 0, *mSurfels->uav());
    mDSurfelVisualDescriptors->setUav(0, 1, *mSurfels->uavCounter().uav());
    mDSurfelVisualDescriptors->setUav(1, 0, *mSurfelVisual->uav());
    
  }
  {
    DescriptorSet::Layout layout;
    layout.addRange(DescriptorSet::Type::StructuredBufferUav, 0, 2);
    layout.addRange(DescriptorSet::Type::TextureUav, 0, 1);

    mDSurfelGIDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDSurfelGIDescriptors->setUav(0, 0, *mSurfels->uav());
    mDSurfelGIDescriptors->setUav(0, 1, *mSurfels->uavCounter().uav());
    mDSurfelGIDescriptors->setUav(1, 0, *mSurfelVisual->uav());

  }
  {
    DescriptorSet::Layout layout;
    layout.addRange(DescriptorSet::Type::StructuredBufferUav, 0, 2);
    layout.addRange(DescriptorSet::Type::TextureUav, 0, 1);

    mDDeferredLightingDescriptors = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

    mDDeferredLightingDescriptors->setUav(0, 0, *mSurfels->uav());
    mDDeferredLightingDescriptors->setUav(0, 1, *mSurfels->uavCounter().uav());
    mDDeferredLightingDescriptors->setUav(1, 0, *mScene->uav());
  }
}

void SceneRenderer::onRenderFrame(RHIContext& ctx) {
  ctx.beforeFrame();
  setupFrame();
  setupView(ctx);
  genGBuffer(ctx);
  genAO(ctx);
  
  computeSurfelCoverage(ctx);
  accumlateSurfels(ctx);
  accumlateGI(ctx);

  if(!Input::Get().isKeyDown(KEYBOARD_SPACE)) {
    deferredLighting(ctx);
  } else {
    visualizeSurfels(ctx);
  }

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

   {
    SCOPED_GPU_EVENT("Gen AccelerationStructure");
    DescriptorSet::Layout layout;

    struct vert_t {
      vec4 position;
      vec4 color;
    };
    mAccelerationStructure = TypedBuffer::create(sizeof(vert_t), totalCount, RHIResource::BindingFlag::UnorderedAccess);
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

  mDGBufferDescriptors->setSrv(0, 5, *ShaderResourceView::nullView());

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
  mDSharedDescriptors->bindForCompute(ctx, *gGenAORootSig, 0);
  mDGBufferDescriptors->bindForCompute(ctx, *gGenAORootSig, 1);
  mDGenAOUavDescriptors->bindForCompute(ctx, *gGenAORootSig, 2);

  uint x = uint(Window::Get()->bounds().width()) / 32 + 1;
  uint y = uint(Window::Get()->bounds().height()) / 32 + 1;

  ctx.dispatch(x, y, 1);

  mDGBufferDescriptors->setSrv(0, 5, mAO->srv());

  ctx.copyResource(*mAO, *RHIDevice::get()->backBuffer());
}

void SceneRenderer::computeSurfelCoverage(RHIContext& ctx) {

  SCOPED_GPU_EVENT("Compute Surfel Coverage");

  static ComputeState::sptr_t computeState;
  if (!computeState) {
    ComputeState::Desc desc;
    desc.setRootSignature(gSurfelCoverageProgram->rootSignature());
    desc.setProgram(gSurfelCoverageProgram);
    computeState = ComputeState::create(desc);
  }
  ctx.resourceBarrier(mGAlbedo.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGNormal.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGPosition.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGDepth.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mAccelerationStructure.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mSurfels.get(), RHIResource::State::UnorderedAccess);
  ctx.resourceBarrier(mSurfelCoverage.get(), RHIResource::State::UnorderedAccess);
  ctx.setComputeState(*computeState);

  // mDGenAOUavDescriptors->setUav(0, 1, mTargetScene.accelerationStructure());
  mDSharedDescriptors->bindForCompute(ctx, *gSurfelCoverageProgram->rootSignature(), 0);
  mDGBufferDescriptors->bindForCompute(ctx, *gSurfelCoverageProgram->rootSignature(), 1);
  mDAccumulateSurfelUavDescriptors->bindForCompute(ctx, *gSurfelCoverageProgram->rootSignature(), 2);

  uint x = uint(Window::Get()->bounds().width()) / 32  + 1;
  uint y = uint(Window::Get()->bounds().height()) / 32 + 1;

  ctx.dispatch(x, y, 1);
  ctx.copyResource(*mSurfelCoverage, *RHIDevice::get()->backBuffer());
}

void SceneRenderer::accumlateSurfels(RHIContext& ctx) {

  SCOPED_GPU_EVENT("Accumlate Surfels");

  static ComputeState::sptr_t computeState;
  if (!computeState) {
    ComputeState::Desc desc;
    desc.setRootSignature(gSurfelPlacementRootSig);
    desc.setProgram(gSurfelPlacementProgram);
    computeState = ComputeState::create(desc);
  }
  ctx.resourceBarrier(mGAlbedo.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGNormal.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGPosition.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGDepth.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mAccelerationStructure.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mSurfels.get(), RHIResource::State::UnorderedAccess);
  ctx.resourceBarrier(mSurfelCoverage.get(), RHIResource::State::UnorderedAccess);

  ctx.setComputeState(*computeState);

  // mDGenAOUavDescriptors->setUav(0, 1, mTargetScene.accelerationStructure());
  mDSharedDescriptors->bindForCompute(ctx, *gSurfelPlacementRootSig, 0);
  mDGBufferDescriptors->bindForCompute(ctx, *gSurfelPlacementRootSig, 1);
  mDAccumulateSurfelUavDescriptors->bindForCompute(ctx, *gSurfelPlacementRootSig, 2);

  uint x = uint(Window::Get()->bounds().width()) / 16 / 32 + 1;
  uint y = uint(Window::Get()->bounds().height()) / 16 / 32 + 1;

  ctx.dispatch(x, y, 1);
  ctx.copyResource(*mSurfelSpawnChance, *RHIDevice::get()->backBuffer());
}

void SceneRenderer::accumlateGI(RHIContext& ctx) {

  SCOPED_GPU_EVENT("Accumlate GI");

  static ComputeState::sptr_t computeState;
  if (!computeState) {
    ComputeState::Desc desc;
    desc.setProgram(gSurfelGIProgram);
    computeState = ComputeState::create(desc);
  }
  ctx.resourceBarrier(mGAlbedo.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGNormal.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGPosition.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGDepth.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mAccelerationStructure.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mSurfels.get(), RHIResource::State::UnorderedAccess);
  ctx.resourceBarrier(mSurfelVisual.get(), RHIResource::State::UnorderedAccess);

  ctx.setComputeState(*computeState);

  // mDGenAOUavDescriptors->setUav(0, 1, mTargetScene.accelerationStructure());
  mDSharedDescriptors->bindForCompute(ctx, *gSurfelGIProgram->rootSignature(), 0);
  mDGBufferDescriptors->bindForCompute(ctx, *gSurfelGIProgram->rootSignature(), 1);
  mDSurfelVisualDescriptors->bindForCompute(ctx, *gSurfelGIProgram->rootSignature(), 2);

  ctx.dispatch(1, 1, 1);
  ctx.copyResource(*mSurfelVisual, *RHIDevice::get()->backBuffer());
}

void SceneRenderer::visualizeSurfels(RHIContext& ctx) {

  SCOPED_GPU_EVENT("Visualize Surfels");

  static ComputeState::sptr_t computeState;
  if (!computeState) {
    ComputeState::Desc desc;
    desc.setRootSignature(gSurfelVisualRootSig);
    desc.setProgram(gSurfelVisualProgram);
    computeState = ComputeState::create(desc);
  }
  ctx.resourceBarrier(mGAlbedo.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGNormal.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGPosition.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mGDepth.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mAccelerationStructure.get(), RHIResource::State::NonPixelShader);
  ctx.resourceBarrier(mSurfels.get(), RHIResource::State::UnorderedAccess);
  ctx.resourceBarrier(mSurfelVisual.get(), RHIResource::State::UnorderedAccess);

  ctx.setComputeState(*computeState);

  // mDGenAOUavDescriptors->setUav(0, 1, mTargetScene.accelerationStructure());
  mDSharedDescriptors->bindForCompute(ctx, *gSurfelVisualRootSig, 0);
  mDGBufferDescriptors->bindForCompute(ctx, *gSurfelVisualRootSig, 1);
  mDSurfelVisualDescriptors->bindForCompute(ctx, *gSurfelVisualRootSig, 2);

  uint x = uint(Window::Get()->bounds().width()) / 32 + 1;
  uint y = uint(Window::Get()->bounds().height()) / 32 + 1;

  ctx.dispatch(x, y, 1);

  ctx.copyResource(*mSurfelVisual, *RHIDevice::get()->backBuffer());

}

void SceneRenderer::deferredLighting(RHIContext& ctx) {
  SCOPED_GPU_EVENT("Deferred Lighting");

  static ComputeState::sptr_t computeState;
  if (!computeState) {
    ComputeState::Desc desc;
    desc.setRootSignature(gDeferredLighting->rootSignature());
    desc.setProgram(gDeferredLighting);
    computeState = ComputeState::create(desc);
  }

  ctx.setComputeState(*computeState);
  mDSharedDescriptors->bindForCompute(ctx, *gDeferredLighting->rootSignature(), 0);
  mDGBufferDescriptors->bindForCompute(ctx, *gDeferredLighting->rootSignature(), 1);
  mDDeferredLightingDescriptors->bindForCompute(ctx, *gDeferredLighting->rootSignature(), 2);

  uint x = uint(Window::Get()->bounds().width()) / 32 + 1;
  uint y = uint(Window::Get()->bounds().height()) / 32 + 1;

  ctx.dispatch(x, y, 1);

  ctx.copyResource(*mScene, *RHIDevice::get()->backBuffer());
}

void SceneRenderer::setupFrame() {

  mFrameData.frameCount++;
  mFrameData.time = (float)GetMainClock().total.second;
  mcFrameData->updateData(mFrameData);

  if(!mTargetScene.lights().empty()) {
    Light* l = mTargetScene.lights()[0];

    mcLight->updateData(l->info());
    
  }
  auto ctx = RHIDevice::get()->defaultRenderContext();
  ctx->clearRenderTarget(mGAlbedo->rtv(), Rgba::black);
  ctx->clearRenderTarget(mGNormal->rtv(), Rgba::gray);
  // ctx->clearRenderTarget(mAO->rtv(), Rgba::white);
  ctx->clearDepthStencilTarget(*mGDepth->dsv(), true, true);
}

void SceneRenderer::setupView(RHIContext& ctx) {
  Camera& cam = *mTargetScene.camera();
  mcCamera->updateData(cam.ubo());

  // TODO: setup viewport later
  ctx.setViewport({ vec2::zero, { Window::Get()->bounds().width(), Window::Get()->bounds().height()} });

}

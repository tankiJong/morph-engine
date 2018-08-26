#include "ImmediateRenderer.hpp"
#include "Engine/Graphics/RHI/RootSignature.hpp"
#include "Engine/Graphics/RHI/FrameBuffer.hpp"
#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Graphics/Program/Material.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Graphics/RHI/PipelineState.hpp"
#include "Engine/Graphics/Model/Mesh.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Engine/Graphics/Model/Vertex.hpp"
#include "Engine/Graphics/Camera.hpp"

void ImmediateRenderer::startUp() {
  {
    FrameBuffer::Desc desc;
    desc.defineColorTarget(0, TEXTURE_FORMAT_RGBA8); // color
    desc.defineColorTarget(1, TEXTURE_FORMAT_RGBA8); // bloom

    desc.defineDepthTarget(TEXTURE_FORMAT_D24S8);

    mFrameBuffer = S<FrameBuffer>(new FrameBuffer(desc));

  }

  mLightBuffer = RHIBuffer::create(sizeof(light_info_t), RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);
  mCameraBuffer = RHIBuffer::create(sizeof(camera_t), RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);
  mModelMatrixBuffer = RHIBuffer::create(sizeof(mat44), RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);

  mProgram.reset(new Program());
  mRhiContext = RHIDevice::get()->defaultRenderContext();

  DescriptorSet::Layout layout;
  // same layout to material, for now, this is hard coded
  layout.addRange(DescriptorSet::Type::Cbv, 0, NUM_UNIFORM_SLOT);
  layout.addRange(DescriptorSet::Type::TextureSrv, 0, NUM_TEXTURE_SLOT);

  mDescriptorSet = DescriptorSet::create(RHIDevice::get()->get()->gpuDescriptorPool(), layout);

  {
    RootSignature::Desc desc;
    desc.addDescriptorSet(layout);
    mRootSignature = RootSignature::create(desc);
  }

  mDescriptorSet->setCbv(0, UNIFORM_LIGHT, *mLightBuffer->cbv());
  mDescriptorSet->setCbv(0, UNIFORM_CAMERA, *mCameraBuffer->cbv());
  mDescriptorSet->setCbv(0, UNIFORM_TRANSFORM, *mModelMatrixBuffer->cbv());
}

void ImmediateRenderer::drawMesh(Mesh& mesh) {
  GraphicsState::Desc desc;

  S<const RootSignature> sig = mProgram->rootSignature();
  if (sig == nullptr) sig = mRootSignature;

  ASSERT_RECOVERABLE(sig != nullptr, "the current root signature is going to be empty");

  if (sig == nullptr) {
    sig = RootSignature::emptyRootSignature();
  }
  desc.setRootSignature(sig);

  mRhiContext->setGraphicsRootSignature(*sig);

  if (mMaterial) {
    mMaterial->bindForGraphics(*mRhiContext, *sig);
  } else {
    mDescriptorSet->bindForGraphics(*mRhiContext, *sig);
  }

  desc.setFboDesc(mFrameBuffer->desc());
  desc.setPrimTye(GraphicsState::PrimitiveType::Triangle);
  desc.setVertexLayout(&mesh.layout());
  desc.setProgram(mProgram);

  GraphicsState::sptr_t gs = GraphicsState::create(desc);

  mRhiContext->setGraphicsState(*gs);

  mRhiContext->setFrameBuffer(*mFrameBuffer);

  mRhiContext->setIndexBuffer(mesh.indices().get());
  for (uint i = 0; i < mesh.layout().attributes().size(); ++i) {
    mRhiContext->setVertexBuffer(*mesh.vertices(i), i);
  }

  for(const draw_instr_t& instr: mesh.instructions()) {
    if(instr.useIndices) {
      mRhiContext->drawIndexed(0, instr.startIndex, instr.elementCount);
    } else {
      mRhiContext->draw(instr.startIndex, instr.elementCount);
    }
  }
}

void ImmediateRenderer::setDepthStencilTarget(const DepthStencilView* dsv) {
  mFrameBuffer->setDepthStencilTarget(dsv);
}

void ImmediateRenderer::setLight(uint index, const light_info_t& lightInfo) {
  EXPECTS(index == 0); // only support 1 light atm
  mLightBuffer->updateData(lightInfo);
}

void ImmediateRenderer::setMaterial(Material& material) {
  mMaterial = &material;
}

void ImmediateRenderer::setModelMatrix(const mat44& model) {
  mModelMatrixBuffer->updateData(&model, 0, sizeof(mat44));
}

void ImmediateRenderer::setProgram(S<Program>& program) {
  mProgram = program;
}

void ImmediateRenderer::setRenderTarget(const RenderTargetView* rtv, uint index) {
  mFrameBuffer->setColorTarget(rtv, index);
}

void ImmediateRenderer::setTexture(eTextureSlot slot, const ShaderResourceView& srv) {
  RHIResource::scptr_t res = srv.res().lock();
  mDescriptorSet->setSrv(1, slot, srv);
}

void ImmediateRenderer::setUniform(eUniformSlot slot, const ConstantBufferView& cbv) {
  RHIResource::scptr_t res = cbv.res().lock();
  mDescriptorSet->setCbv(0, slot, cbv);
}

void ImmediateRenderer::setView(const Camera& cam) {
  mRhiContext->setViewport(aabb2{ vec2::zero, vec2(cam.width(), cam.height()) });
  mCameraBuffer->updateData(&cam.ubo(), 0, sizeof(camera_t));
}

const DepthStencilView* ImmediateRenderer::defaultDsv() const {
  return RHIDevice::get()->depthBuffer()->dsv();
}

const RenderTargetView& ImmediateRenderer::defaultRtv() const {
  return RHIDevice::get()->backBuffer()->rtv();
}

ImmediateRenderer* gImmediateRenderer = nullptr;

ImmediateRenderer& ImmediateRenderer::get() {
  if(gImmediateRenderer == nullptr) {
    gImmediateRenderer = new ImmediateRenderer();
  }

  return *gImmediateRenderer;
}

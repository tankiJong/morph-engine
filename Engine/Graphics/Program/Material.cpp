#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Material.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

Material::Prop::Prop() {
  buffer = RHIBuffer::create(1, RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);
}

void Material::init() {
  DescriptorSet::Layout layout;

  // after I have the shader reflaction, the layout will be on the shader

  // **IMPORTANT** this layout match with the Rootsignature defined in HLSL

  layout.addRange(DescriptorSet::Type::Cbv, UNIFORM_USER_1, NUM_MAT_CBV);
  layout.addRange(DescriptorSet::Type::TextureSrv, TEXTURE_DIFFUSE, NUM_MAT_TEXTURE);

  mConstBufferRegisterBase = UNIFORM_USER_1;

  mConstProperties.resize(NUM_MAT_CBV);
  for(uint i = 0; i < mConstProperties.size(); i++) {
    mConstProperties[i].slot = (eUniformSlot)(UNIFORM_USER_1 + i);
  }

  mDescriptorSet = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);
}

void Material::setTexture(eTextureSlot prop, Texture2::sptr_t tex) {
  mPropertyTextures[prop - MAT_TEXTURE_SLOT_START] = tex;
}

void Material::setProperty(eUniformSlot registerIndex, const void* data, size_t size) {
  Prop* p = nullptr;

  for(Prop& prop: mConstProperties) {
    if(prop.slot == registerIndex) {
      p = &prop;
      break;
    }
  }

  // material does not support expending params on the fly.
  EXPECTS(p != nullptr);

  p->buffer->updateData(data, 0, size);
}

void Material::bindForGraphics(RHIContext& ctx, const RootSignature& root, uint rootIndex) const {
  finialize(ctx);
  mDescriptorSet->bindForGraphics(ctx, root, rootIndex);
}

void Material::bindForCompute(RHIContext& ctx, const RootSignature& root, uint rootIndex) const {
  finialize(ctx);
  mDescriptorSet->bindForCompute(ctx, root, rootIndex);
}

void Material::finialize(RHIContext& ctx) const {
  for(const Prop& prop: mConstProperties) {
    mDescriptorSet->setCbv(0, prop.slot - UNIFORM_USER_1, *prop.buffer->cbv());
  }

  for(uint i = 0; i < NUM_MAT_TEXTURE; i++) {
    if(mPropertyTextures[i] != nullptr) {
      auto* srv = mPropertyTextures[i]->srv();
      ASSERT_RECOVERABLE(srv != nullptr, "try to bind nullptr srv for material");
      mDescriptorSet->setSrv(1, i, *srv);
      ctx.transitionBarrier(mPropertyTextures[i].get(), RHIResource::State::ShaderResource);
    } else {
      mDescriptorSet->setSrv(1, i, *ShaderResourceView::nullView());
    }
  }
}

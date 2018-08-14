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

void Material::init(uint cboRegisterIndexStart, uint cboRegisterCount) {
  DescriptorSet::Layout layout;

  // after I have the shader reflaction, the layout will be on the shader
  layout.addRange(DescriptorSet::Type::TextureSrv, TEXTURE_DIFFUSE, 3);
  if(cboRegisterCount != 0) {
    layout.addRange(DescriptorSet::Type::Cbv, cboRegisterIndexStart, cboRegisterCount);
    mConstBufferRegisterBase = cboRegisterIndexStart;
  }

  mConstProperties.resize(cboRegisterCount);
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

void Material::bindForGraphics(const RHIContext& ctx, const RootSignature& root, uint rootIndex) {
  finialize();
  mDescriptorSet->bindForGraphics(ctx, root, rootIndex);
}

void Material::bindForCompute(const RHIContext& ctx, const RootSignature& root, uint rootIndex) {
  finialize();
  mDescriptorSet->bindForCompute(ctx, root, rootIndex);
}

void Material::finialize() {
  for(uint i = 0; i < NUM_TEXTURE_SLOT; i++) {
    mDescriptorSet->setSrv(0, i + MAT_TEXTURE_SLOT_START, mPropertyTextures[i]->srv());
  }

  for(Prop& prop: mConstProperties) {
    mDescriptorSet->setCbv(1, prop.slot - mConstBufferRegisterBase, *prop.buffer->cbv());
  }
}

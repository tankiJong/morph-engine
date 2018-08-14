#pragma once
#include "Engine/Core/common.hpp"


/*
 * Shader binding:
 * 
 * (Reference to eUniformSlot)
 * 
 * The whole Root Signature layouted like this:
  [0]Descriptor Set - Per Frame Data - Update in Begin Frame
  b0: UNIFORM_TIME,

  [1]Descriptor Set - Per View Data - Update when switch camera
  b1: UNIFORM_CAMERA,
  t0: TEXTURE_AO

  [2]Descripotr Set - Per Instance Data - update on each draw call, live on Material
  b2: UNIFORM_TRANSFORM,
  b3: UNIFORM_LIGHT,
  t1: TEXTURE_DIFFUSE,
  t2: TEXTURE_NORMAL,
  t3: TEXTURE_SPECULAR,
 * 
 * 
 * 
 */


class Material {
  static constexpr uint NUM_MAT_TEXTURE = TEXTURE_SPECULAR - TEXTURE_DIFFUSE + 1;
  static constexpr uint MAT_TEXTURE_SLOT_START = TEXTURE_DIFFUSE;
  struct Prop {
    eUniformSlot slot;
    std::string name;
    RHIBuffer::sptr_t buffer;

    Prop();
  };
public:
  void init(uint cboRegisterIndexStart = UNIFORM_USER_1, uint cboRegisterCount = 0);
  void setTexture(eTextureSlot prop, Texture2::sptr_t tex);
  void setProperty(eUniformSlot registerIndex, const void* data, size_t size);
  void bindForGraphics(const RHIContext& ctx, const RootSignature& root, uint rootIndex = 0);
  void bindForCompute(const RHIContext& ctx, const RootSignature& root, uint rootIndex = 0);

protected:
  void finialize();
  uint mConstBufferRegisterBase = uint(-1);
  std::array<Texture2::sptr_t, NUM_MAT_TEXTURE> mPropertyTextures;
  std::vector<Prop> mConstProperties;
  DescriptorSet::sptr_t mDescriptorSet;
};

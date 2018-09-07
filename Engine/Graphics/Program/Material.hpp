#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIType.hpp"

class RHIBuffer;
class DescriptorSet;
class Texture2;
class RHIContext;
class RootSignature;
class Material {
  static constexpr uint NUM_MAT_TEXTURE = TEXTURE_USER_2 - TEXTURE_DIFFUSE + 1;
  static constexpr uint MAT_TEXTURE_SLOT_START = TEXTURE_DIFFUSE;

  static constexpr uint NUM_MAT_CBV = NUM_UNIFORM_SLOT - UNIFORM_USER_1;
  struct Prop {
    eUniformSlot slot;
    std::string name;
    S<RHIBuffer> buffer;

    Prop();
  };
public:
  void init();
  void setTexture(eTextureSlot prop, S<Texture2> tex);
  void setProperty(eUniformSlot registerIndex, const void* data, size_t size);
  void bindForGraphics(RHIContext& ctx, const RootSignature& root, uint rootIndex = 0) const;
  void bindForCompute(RHIContext& ctx, const RootSignature& root, uint rootIndex = 0) const;

protected:
  void finialize(RHIContext& ctx) const;
  uint mConstBufferRegisterBase = uint(-1);
  std::array<S<Texture2>, NUM_MAT_TEXTURE> mPropertyTextures;
  std::vector<Prop> mConstProperties;
  mutable S<DescriptorSet> mDescriptorSet;
};

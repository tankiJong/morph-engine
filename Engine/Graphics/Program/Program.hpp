#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/Shader.hpp"


class ProgramIns;

class Program: public std::enable_shared_from_this<Program> {
public:
  using sptr_t = S<Program>;
  Program();
  Shader& stage(eShaderType type) {
    return mShaders[type];
  }

  const Shader& stage(eShaderType type) const {
    return mShaders[type];
  }
  void setRenderState(const RenderState& state);
  bool compile();

  const RenderState& renderState() const;
  S<const RootSignature> rootSignature() const;
  span<const DescriptorSet::Layout> descriptorLayout() const { return mLayouts; };
  span<DescriptorSet::Layout> descriptorLayout() { return mLayouts; };
  // void setDescriptorLayout(const DescriptorSet::Layout& layout);

  S<ProgramIns> instantiate();

protected:
  std::array<Shader, NUM_SHADER_TYPE> mShaders;
  std::vector<DescriptorSet::Layout> mLayouts;
  RenderState mRenderState;
  S<const RootSignature> mRootSig;
  bool mIsDirty = true;
};

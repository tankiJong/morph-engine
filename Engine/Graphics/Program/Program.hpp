#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/Shader.hpp"

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

  RenderState& renderState();
  S<const RootSignature> rootSignature() const;
  DescriptorSet::Layout& descriptorLayout() { return mLayout; };
  const DescriptorSet::Layout& descriptorLayout() const { return mLayout; };
  // void setDescriptorLayout(const DescriptorSet::Layout& layout);

protected:
  std::array<Shader, NUM_SHADER_TYPE> mShaders;
  DescriptorSet::Layout mLayout;
  RenderState mRenderState;
  S<const RootSignature> mRootSig;
  bool mIsDirty = true;
};

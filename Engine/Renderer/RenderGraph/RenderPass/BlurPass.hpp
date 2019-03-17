#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/RenderGraph/RenderPass.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"
#include "Engine/Renderer/RenderGraph/RenderResourceHandle.hpp"

class RenderNodeContext;
class RHIContext;

class BlurPass: public RenderPass {
public:
  static constexpr const char* kInput = "target";
  static constexpr const char* kOutput = "result";

  BlurPass(bool vertical);

  void construct(RenderNodeBuilder& builder, RenderNodeContext& context) override;
  void execute(const RenderGraphResourceSet& set, RHIContext& ctx) const override;

protected:
  RenderResourceHandle* mTarget;
  RHIBuffer::sptr_t mcBlurConstant;
  RenderResourceHandle* mTempBuffer;
  struct BlurConstant {
    float verticalFlag;
    vec3 __padding;
  };

  BlurConstant mBlurConstant;
};

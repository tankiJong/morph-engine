#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/RenderGraph/RenderPass.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"

class RenderNodeContext;
class RHIContext;

class BlurPass: public RenderPass {
public:
  BlurPass(const Texture2::sptr_t& target, bool vertical);

  void construct(RenderNodeContext& builder) override;
  void execute(RHIContext& ctx) const override;

protected:
  Texture2::sptr_t mTarget;
  Texture2::sptr_t mTempBuffer;
  RHIBuffer::sptr_t mcBlurConstant;

  struct BlurConstant {
    float verticalFlag;
    vec3 __padding;
  };

  BlurConstant mBlurConstant;
};

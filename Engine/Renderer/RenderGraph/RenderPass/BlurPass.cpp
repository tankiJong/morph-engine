#include "BlurPass.hpp"
#include "Engine/Graphics/Program/Program.hpp"

#include "Renderer/RenderGraph/RenderPass/Blur_cs.h"
#include "Engine/Renderer/RenderGraph/RenderNodeContext.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"

BlurPass::BlurPass(const Texture2::sptr_t& target, bool vertical) {
  mBlurConstant.verticalFlag = vertical ? 1.f : 0.f;

  mcBlurConstant = RHIBuffer::create(sizeof(BlurConstant), RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);
  mcBlurConstant->updateData(mBlurConstant);
  mTarget = target;
  mTempBuffer = Texture2::create(mTarget->width(), mTarget->height(), 
                                 mTarget->format(), 
                                 RHIResource::BindingFlag::ShaderResource |
                                 RHIResource::BindingFlag::UnorderedAccess |
                                 RHIResource::BindingFlag::RenderTarget);
}

void BlurPass::construct(RenderNodeContext& builder) {
  auto prog = Resource<Program>::get("internal/ShaderPass/Blur");

  builder.reset(prog, true);

  builder.readCbv("blur-constant", mcBlurConstant, 0);
  builder.readSrv("blur-input", mTarget, 0);
  builder.readWriteUav("blur-output", mTempBuffer, 0);
}

void BlurPass::execute(RHIContext& ctx) const {
  uint width = mTarget->width();
  uint height = mTarget->height();

  ctx.dispatch(width / 16 + 1, height / 16 + 1, 1);
  ctx.copyResource(*mTempBuffer, *mTarget);
}

DEF_RESOURCE(Program, "internal/ShaderPass/Blur") {
  Program::sptr_t prog(new Program());

  prog->stage(SHADER_TYPE_COMPUTE).setFromBinary(gBlur_cs, sizeof(gBlur_cs));
  prog->compile();

  return prog;
}
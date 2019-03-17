#include "BlurPass.hpp"
#include "Engine/Graphics/Program/Program.hpp"

#include "Renderer/RenderGraph/RenderPass/Blur_cs.h"
#include "Engine/Renderer/RenderGraph/RenderNodeContext.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Renderer/RenderGraph/RenderNodeBuilder.hpp"
#include "Engine/Renderer/RenderGraph/RenderGraphResourceSet.hpp"

BlurPass::BlurPass(bool vertical) {
  mBlurConstant.verticalFlag = vertical ? 1.f : 0.f;

  mcBlurConstant = RHIBuffer::create(sizeof(BlurConstant), RHIResource::BindingFlag::ConstantBuffer, RHIBuffer::CPUAccess::Write);
  mcBlurConstant->updateData(mBlurConstant);
  // mTempBuffer = Texture2::create(mTarget->width(), mTarget->height(), 
  //                                mTarget->format(), 
  //                                RHIResource::BindingFlag::ShaderResource |
  //                                RHIResource::BindingFlag::UnorderedAccess |
  //                                RHIResource::BindingFlag::RenderTarget);
}

void BlurPass::construct(RenderNodeBuilder& builder, RenderNodeContext& context) {
  
  auto& target = builder.inputOutput<Texture2>(kInput);
  mTarget = &target;

  auto prog = Resource<Program>::get("internal/ShaderPass/Blur");

  context.reset(prog, true);

  auto& constants = context.bind<RHIBuffer>(mcBlurConstant, "blur-constant");

  RenderGraphResourceDesc desc;
  desc.type = RHIResource::Type::Texture2D;
  desc.texture2.format = TEXTURE_FORMAT_RGBA8;
  auto& tempBuffer = context.extend(target, "temp-buffer", desc);

  mTempBuffer = &tempBuffer;

  context.readCbv(".blur-constant", 0);
  context.readSrv(".target", 0);
  context.readWriteUav(tempBuffer, 0);
}

void BlurPass::execute(const RenderGraphResourceSet& set, RHIContext& ctx) const {
  auto tempBuffer = set.get<Texture2>(*mTempBuffer);
  uint width = tempBuffer->width();
  uint height = tempBuffer->height();

  ctx.dispatch(width / 16 + 1, height / 16 + 1, 1);

  auto target = set.get<Texture2>(*mTarget);

  ctx.copyResource(*tempBuffer, *target);
}

DEF_RESOURCE(Program, "internal/ShaderPass/Blur") {
  Program::sptr_t prog(new Program());

  prog->stage(SHADER_TYPE_COMPUTE).setFromBinary(gBlur_cs, sizeof(gBlur_cs));
  prog->compile();

  return prog;
}
#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"

class RenderNodeContext;
class RenderPassExecutor;


class RenderPass {
public:
  virtual ~RenderPass() = default;

  virtual void construct(RenderNodeContext& builder) = 0;
  virtual void execute(RHIContext& ctx) const = 0;
};

#pragma once
#include "Engine/Core/common.hpp"

class RenderNodeContext;
class RHIContext;

class RenderPass {
public:
  virtual ~RenderPass() = default;
  
  virtual void construct(RenderNodeContext& builder) = 0;
  virtual void execute(RHIContext& ctx) const = 0;
};

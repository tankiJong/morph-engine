#pragma once
#include "Engine/Core/common.hpp"

class RenderNodeContext;
class RenderNodeBuilder;
class RenderGraphResourceSet;
class RHIContext;

class RenderPass {
public:
  virtual ~RenderPass() = default;
  
  virtual void construct(RenderNodeBuilder& builder, RenderNodeContext& context) = 0;
  virtual void execute(const RenderGraphResourceSet& set, RHIContext& ctx) const = 0;
};

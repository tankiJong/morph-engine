#pragma once

#include "Engine/Core/common.hpp"
#include <functional>

class RenderPass;
class RHIContext;
class RenderGraphBuilder;

class RenderPassExecutor: std::function<void(RHIContext&)> {
public:
  template<typename Lambda>
  RenderPassExecutor(Lambda&& lambda): std::function<void(RHIContext&)>(std::forward(lambda)) {};
protected:
  using BaseType = std::function<void(RHIContext&)>;
};

class RenderGraph {
public:
  // using RenderPassExecutor = std::function<void(CommandList&)>;
  using RenderPassConstructor = std::function<RenderPassExecutor(RenderGraphBuilder&)>;

  RenderPass& definePass(std::string_view name, RenderPassConstructor constructor);
};
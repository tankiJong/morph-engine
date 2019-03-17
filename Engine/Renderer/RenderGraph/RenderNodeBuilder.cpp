#include "RenderNodeBuilder.hpp"
#include "Engine/Renderer/RenderGraph/RenderNode.hpp"
#include "Engine/Renderer/RenderGraph/RenderResourceHandle.hpp"
#include "Engine/Renderer/RenderGraph/RenderGraphResourceSet.hpp"

const RenderResourceHandle& RenderNodeBuilder::input(std::string name, ResourceType rhiType, const unique& type) {

  name = mNode->name() + "." + name;
  RenderResourceHandle& resHandle = mNode->resourceSet().declare(name, type, rhiType);

  Param param {
    name,
    IoType::Input,
    rhiType,
    &resHandle,
  };

  auto [iter, result] = mInputs.emplace(name, param);

  EXPECTS(result);

  return resHandle;
}

RenderResourceHandle& RenderNodeBuilder::output(std::string name, const RenderGraphResourceDesc& desc, ResourceType rhiType, const unique& type) {
  name = mNode->name() + "." + name;

  RenderResourceHandle& resHandle = mNode->resourceSet().declare(name, desc, type);

  Param param {
    name,
    IoType::Output,
    rhiType,
    &resHandle,
  };

  auto [iter, result] = mOutputs.emplace(name, param);

  EXPECTS(result);

  return resHandle;
}

RenderResourceHandle& RenderNodeBuilder::inputOutput(std::string name, ResourceType rhiType, const unique& type) {
  name = mNode->name() + "." + name;

  RenderResourceHandle& resHandle = mNode->resourceSet().declare(name, type, rhiType);

  Param param {
    name,
    IoType::Output | IoType::Input,
    rhiType,
    &resHandle,
  };

  auto [_,  result1] = mOutputs.emplace(name, param);
  auto [__, result2] = mInputs.emplace(name, param);

  EXPECTS(result1 && result2);

  return resHandle;
}

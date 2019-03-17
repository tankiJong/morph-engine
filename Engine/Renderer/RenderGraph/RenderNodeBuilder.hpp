#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Renderer/RenderGraph/RenderResourceHandle.hpp"
#include <map>
class RenderNode;

class RenderNodeBuilder {

public:
  enum class IoType: uint {
    Input,
    Output,
  };

  using ResourceType = RHIResource::Type;

  struct Param {
    std::string name;
    IoType ioType;
    RHIResource::Type rhiType;
    RenderGraphResourceHandle* handle;
  };

  RenderNodeBuilder(RenderNode* node): mNode(node) {}

  template<
    typename T, 
    typename = std::enable_if_t<!rhi_type_t<T>::valid>>
  const RenderGraphResourceHandle& input(std::string name) {
    return input(name, rhi_type_t<T>::value, tid<T>::value);
  };

  template<
    typename T, 
    typename = std::enable_if_t<rhi_type_t<T>::valid>>
  const RenderResourceHandle& input(std::string name) {
    return input(name, rhi_type_t<T>::value, tid<T>::value);
  };

  //   template<typename T>
  // auto input(std::string name) 
  //                    -> std::conditional_t<
  //                       rhi_type_t<T>::valid, 
  //                       RenderResourceHandle&, 
  //                       RenderGraphResourceHandle&> {
  //   return input(name, rhi_type_t<T>::value, tid<T>::value);
  // };

  template<typename T>
  const RenderGraphResourceHandle& output(std::string name, RenderGraphResourceDesc desc) {
    RenderResourceHandle& handle = output(name, desc, rhi_type_t<T>::value, tid<T>::value);
    return handle;
  };

  template<typename T>
  auto inputOutput(std::string name)
                            -> std::conditional_t<
                               rhi_type_t<T>::valid, 
                               RenderResourceHandle&, 
                               RenderGraphResourceHandle&> {
    RenderResourceHandle& handle = inputOutput(name, rhi_type_t<T>::value, tid<T>::value);
    return handle;
  };
protected:

  const RenderResourceHandle& input(
    std::string name, ResourceType rhiType, const unique& type);
  RenderResourceHandle& output(
    std::string name, const RenderGraphResourceDesc& desc, ResourceType rhiType, const unique& type);
  RenderResourceHandle& inputOutput(
    std::string name, ResourceType rhiType, const unique& type);
  std::map<std::string, Param, std::less<>> mInputs;
  std::map<std::string, Param, std::less<>> mOutputs;

  RenderNode* mNode = nullptr;
};

enum_class_operators(RenderNodeBuilder::IoType);

#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include "Engine/Renderer/RenderGraph/RenderResourceHandle.hpp"

class RenderNode;

struct RenderEdge {
  using ResourceState = RHIResource::State;
  struct BindingInfo {
    std::string name;
    const RenderResourceHandle* handle;
    ResourceState state;

    uint regIndex;
    uint regSpace;
    // bool isOwned;
  };


  //----- data members -----
  RenderNode* from = nullptr;
  RenderNode* to = nullptr;
  BindingInfo* fromRes = nullptr;
  BindingInfo* toRes = nullptr;

  bool operator==(const RenderEdge& rhs) const {
    return from == rhs.from
        && to == rhs.to
        && fromRes == rhs.fromRes
        && toRes == rhs.toRes;
  }
};

namespace std {
  template<> struct hash<RenderEdge> {
    size_t operator()(const RenderEdge& e) const noexcept {
      auto hasher = std::hash<void*>();

      size_t a1 = hasher(e.from);
      size_t a2 = hasher(e.to);
      size_t a3 = hasher(e.fromRes);
      size_t a4 = hasher(e.toRes);

      return (a1 & a2 >> 16) & a3 + a4;
    }

    // template<> struct equal_to<RenderEdge> {
    //   bool operator()(const RenderEdge& lhs, const RenderEdge& rhs) const noexcept{
    //     return lhs == rhs;
    //   }
    // };
  };
}

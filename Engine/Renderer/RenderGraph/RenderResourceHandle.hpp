#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIType.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include <optional>
#include "Engine/Graphics/RHI/RHIBuffer.hpp"
#include "Engine/Math/Primitives/uvec2.hpp"

using render_graph_res_handle_t = uint;
class RenderResourceHandle;




struct RenderGraphResourceDesc {
  RHIResource::Type type = RHIResource::Type::Buffer;
  std::optional<RHIResource::BindingFlag> bindingFlags = std::nullopt;
  struct Texture2Desc {
    std::optional<eTextureFormat> format = std::nullopt;
    std::optional<uvec2> size = std::nullopt;

    Texture2Desc& operator=(const Texture2Desc& copy) = default;
    bool operator==(const Texture2Desc& rhs) const;
  } texture2;

  struct BufferDesc {
    std::optional<size_t> size = std::nullopt;
    std::optional<RHIBuffer::CPUAccess> cpuAccess = std::nullopt;
    BufferDesc& operator=(const BufferDesc& copy) = default;
    bool operator==(const BufferDesc& rhs) const;

  } buffer;

  RenderGraphResourceDesc& operator=(const RenderGraphResourceDesc& copy) = default;
  bool operator==(const RenderGraphResourceDesc& rhs) const;
};





class RenderGraphResourceHandle {
  friend class RenderGraphResourceSet;
public:
  enum eOwnership {
    OWN_RESOURCESET,
    OWN_EXTERNAL,
  };

  enum eHandleResourceType {
    HANDLE_RES_GENERAL,
    HANDLE_RES_RHI,
  };
  std::string name;
  RenderGraphResourceHandle() : id(sNextId++) {};

  RenderGraphResourceHandle(const RenderGraphResourceHandle&) = default;

  RenderResourceHandle* cast() {
    if(resourceType == HANDLE_RES_RHI) {
      return (RenderResourceHandle*)(this);
    }
    return nullptr;
  }

  const RenderResourceHandle* cast() const {
    if(resourceType == HANDLE_RES_RHI) {
      return (const RenderResourceHandle*)(this);
    }
    return nullptr;
  }

protected:
  static constexpr render_graph_res_handle_t INVALID_HANDLE_ID = 0xffffffff;
  static uint sNextId;

  const unique* type = nullptr;
  eOwnership owner = OWN_RESOURCESET;
  render_graph_res_handle_t id = INVALID_HANDLE_ID;
  render_graph_res_handle_t parentId = INVALID_HANDLE_ID;
  eHandleResourceType resourceType = HANDLE_RES_GENERAL;
};

class RenderResourceHandle: public RenderGraphResourceHandle {
  friend class RenderGraphResourceSet;
public:
  RenderGraphResourceDesc desc;

  RenderResourceHandle() : RenderGraphResourceHandle() {
    resourceType = HANDLE_RES_RHI;
  };
  RenderResourceHandle(const unique& t, eOwnership owner);

protected:
  RenderResourceHandle(const RenderResourceHandle&) = default;
  RenderResourceHandle& operator=(const RenderResourceHandle&) = default;
  RenderResourceHandle extend() const {
    RenderResourceHandle handle(*type, owner);
    handle.parentId = id;
    return handle;
  }
  
};



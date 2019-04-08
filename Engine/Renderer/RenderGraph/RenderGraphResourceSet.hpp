#pragma once
#include "Engine/Core/common.hpp"
#include <map>
#include <any>
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"
#include "Engine/Renderer/RenderGraph/RenderResourceHandle.hpp"

class RenderNode;
class RenderResourceHandle;

template<typename RHIType, bool IsRHIType = rhi_type_t<RHIType>::valid>
struct rhi_sptr;

template<typename RHIType>
struct rhi_sptr<RHIType, true> {
  using type = std::conditional_t<std::is_const_v<RHIType>, typename RHIType::scptr_t, typename RHIType::sptr_t>;
};

template<typename RHIType>
struct rhi_sptr<RHIType, false> {
  using type = std::add_pointer_t<RHIType>;
};

template<typename T>
using rhi_sptr_t = typename rhi_sptr<T>::type;


class RenderGraphResourceSet {
  friend class RenderNodeBuilder;
public:

  const Texture2::sptr_t& backBuffer() const;

  template<typename T>
  auto get(const RenderGraphResourceHandle& handle) const -> rhi_sptr_t<T>;

  template< typename T >
  decltype(auto) get(std::string_view name) const {
    const RenderGraphResourceHandle* handle = find(name);
    return get<T>(*handle);
  }

  template<
    typename T, 
    typename = std::enable_if_t<rhi_type_t<T>::valid>>
  const RenderResourceHandle& input(std::string name) {
    return input(name, rhi_type_t<T>::value, tid<T>::value);
  };

  template< typename T, typename ResType = rhi_sptr_t<T>>
  void set(ResType&& res, 
           const RenderGraphResourceHandle& handle, 
           RenderGraphResourceHandle::eOwnership ownership /*= RenderGraphResourceHandle::OWN_RESOURCESET*/,
           bool isStatic);

  template<typename T>
  RenderResourceHandle& declare(std::string name, const RenderGraphResourceDesc& desc) {
    static_assert(std::is_base_of_v<RHIResource, T>, "has to be rhi resource type");
    return declare(name, desc, tid<T>::value);
  }
  template<typename T>
  decltype(auto) declare(std::string name) {
    if constexpr(rhi_type_t<T>::valid) {
      return declare(name, tid<T>::value, rhi_type_t<T>::value);
    } else {
      return declare(name, tid<T>::value);
    }
  }

  void clearResources();

  RenderResourceHandle& extend(const RenderResourceHandle& source, const std::string& name, const RenderGraphResourceDesc& desc);
  RenderResourceHandle& extend(const std::string& source, const std::string& name, const RenderGraphResourceDesc& desc);
  const RenderGraphResourceHandle* find(std::string_view name) const;
  RenderGraphResourceHandle* find(std::string_view name);

  /*
  * `from` -> `to`, 
  * `from` depends on `to`, 
  * `from` is derived from `to`
  */ 
  void bind(std::string_view from, std::string_view to);

  void setBackbuffer(const Texture2::sptr_t& backbuffer);

protected:

  static constexpr uint INVALID_NODE_ID = 0xabcdabcd;

  enum eResolvePolicy {
    RESOLVE_FROM_PARENT_CREATE,
    RESOLVE_FROM_PARENT_REFERENCE,
    RESOLVE_FROM_CREATE,
    RESOLVE_FROM_EXTERNAL,
  };

  class ResourceCache {
  public:
    U<RenderGraphResourceHandle> handle = nullptr;
    std::any res = {};
    uint nodeId = INVALID_NODE_ID;
    bool isStatic = false;
    bool ready() const { return res.has_value(); }

  };

  class ResourceNode {
  public:
    static uint sNextUsableId;
    
    ResourceCache* content;
    eResolvePolicy resolvePolicy = RESOLVE_FROM_CREATE;
    uint id;

    ResourceNode(ResourceCache* c)
      : content(c), id(sNextUsableId++) {
      content->nodeId = id;
    }

    void addIncoming(ResourceNode* node);
    void setOutgoing(ResourceNode* node);

    ResourceNode(ResourceNode&&) = default;
    ResourceNode& operator=(ResourceNode&&) = default;
  protected:
    ResourceNode* mOutgoingNode = nullptr;
    std::vector<ResourceNode*> mIncomingNodes;

  private:

    ResourceNode(const ResourceNode&) = default;
    ResourceNode& operator=(const ResourceNode&) = default;

  };

  class ResourceGraph {
  public:
    void add(ResourceCache* content);

    /*
     * `from` -> `to`, 
     * `from` depends on `to`, 
     * `from` is derived from `to`
     */ 
    void connect(uint from, uint to, bool isReference);
    
    ResourceNode* node(const ResourceCache& cache) {
      return node(cache.nodeId);
    }

    const ResourceNode* node(const ResourceCache& cache) const {
      return node(cache.nodeId);
    }

  protected:
    ResourceNode* node(uint id);
    const ResourceNode* node(uint id) const;
    std::map<uint, ResourceNode> mNodes;
  };

  ResourceCache* allocResouceCache(uint handleId);
  void freeResourceCache(ResourceCache* res);

  // this is for RHI resources, and will potentially resolve all parent resources
  void resolveResource(ResourceCache& target) const;

  RenderGraphResourceHandle& declare(std::string name, const unique& type);
  RenderResourceHandle& declare(std::string name, const unique& type, RHIResource::Type rhiType);
  RenderResourceHandle& declare(std::string name, const RenderGraphResourceDesc& desc, const unique& type);

  Texture2::sptr_t mCustomBackBuffer;
  ResourceGraph mResourceGraph; // graph of ResourceNode hierarchy
  std::map<render_graph_res_handle_t, ResourceCache*> mResourceMap;
};

template< typename T>
auto RenderGraphResourceSet::get(const RenderGraphResourceHandle& handle) const 
  -> rhi_sptr_t<T> {
  auto iter = mResourceMap.find(handle.id);
  ResourceCache* cache = iter->second;

  if constexpr(rhi_type_t<T>::valid) {
    resolveResource(*cache);
    EXPECTS(cache->ready());
    auto rhiHandle = handle.cast();
    if constexpr(std::is_same_v<T, RHIResource>) {
      RHIResource::sptr_t ptr;
      switch(rhiHandle->desc.type) {
        case RHIResource::Type::Buffer: {
          ptr = std::any_cast<RHIBuffer::sptr_t>(cache->res);
        }
        break;
        case RHIResource::Type::Texture2D: {
          ptr = std::any_cast<Texture2::sptr_t>(cache->res);
        }
        break;
        case RHIResource::Type::TextureCube: {
          ptr = std::any_cast<TextureCube::sptr_t>(cache->res);
        }
        break;
        default:
        BAD_CODE_PATH();
      }
      return ptr;
    } else if constexpr(std::is_same_v<T, const RHIResource>) {
      RHIResource::scptr_t ptr;
      switch(rhiHandle->desc.type) {
        case RHIResource::Type::Buffer: {
          auto* rptr = std::any_cast<RHIBuffer::scptr_t>(&cache->res);
          if(rptr == nullptr) {
            ptr = std::any_cast<RHIBuffer::sptr_t>(cache->res);
          } else {
            ptr = *rptr;
          }
        }
        break;
        case RHIResource::Type::Texture2D: {
          auto* rptr = std::any_cast<Texture2::scptr_t>(&cache->res);
          if(rptr == nullptr) {
            ptr = std::any_cast<Texture2::sptr_t>(cache->res);
          } else {
            ptr = *rptr;
          }
        }
        break;
        case RHIResource::Type::TextureCube: {
          auto* rptr = std::any_cast<TextureCube::scptr_t>(&cache->res);
          if(rptr == nullptr) {
            ptr = std::any_cast<TextureCube::sptr_t>(cache->res);
          } else {
            ptr = *rptr;
          }
        }
        break;
        default:
        BAD_CODE_PATH();
      }
      return ptr;
    } else {
      return std::any_cast<rhi_sptr_t<T>>(cache->res);
    }
  } else {
    EXPECTS(cache->ready());
    return std::any_cast<rhi_sptr_t<T>>(cache->res);
  }
  // EXPECTS(*cache->handle->type == tid<T>::value);
  BAD_CODE_PATH();
}

template< typename T, typename ResType>
void RenderGraphResourceSet::set(ResType&& res, const RenderGraphResourceHandle& handle, 
                                 RenderGraphResourceHandle::eOwnership ownership, bool isStatic) {
  auto iter = mResourceMap.find(handle.id);
  ResourceCache* cache = iter->second;
  cache->isStatic = isStatic;
  const unique& t = tid<T>::value;
  EXPECTS(*cache->handle->type == t);
  // EXPECTS(!cache->ready());

  cache->res = res;
  cache->handle->owner = ownership;

#ifdef _DEBUG
  auto* node = mResourceGraph.node(*cache);
  EXPECTS(node->resolvePolicy == RESOLVE_FROM_CREATE);
#endif
}

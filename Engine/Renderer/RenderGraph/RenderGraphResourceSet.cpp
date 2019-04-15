#include "RenderGraphResourceSet.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

class ResourcePool {
protected:
  struct Cache {
    RenderGraphResourceDesc desc;
    RHIResource::sptr_t res;
  };

  std::vector<Cache> mAvailableCache;

  template<typename RHIType>
  typename RHIType::sptr_t create(const RenderGraphResourceDesc& desc) {
    if constexpr(std::is_same_v<RHIType, RHIBuffer>) {
      EXPECTS(desc.type == RHIResource::Type::Buffer);
      auto res = RHIBuffer::create(*desc.buffer.size, *desc.bindingFlags, *desc.buffer.cpuAccess);
	  //mAvailableCache.emplace_back(Cache{desc, res});
      return res;
    }

    if constexpr(std::is_same_v<RHIType, Texture2>) {
      EXPECTS(desc.type == RHIResource::Type::Texture2D)
      auto res = Texture2::create(desc.texture2.size->x, desc.texture2.size->y,
                                    *desc.texture2.format, *desc.bindingFlags);
	  //mAvailableCache.emplace_back(Cache{desc, res});
      return res;
    }

    BAD_CODE_PATH();
    return nullptr;
  }

public:
  template<typename RHIType>
  typename RHIType::sptr_t acquire(const RenderGraphResourceDesc& desc) {
    using sptr_t = typename RHIType::sptr_t;
    for(size_t i = 0; i < mAvailableCache.size(); i++) {
      Cache& c = mAvailableCache[i];
      if(desc == c.desc) {
        RHIResource::sptr_t res = c.res;
        std::swap(c, mAvailableCache.back());
        mAvailableCache.pop_back();
        return std::dynamic_pointer_cast<RHIType>(res);
      }
    }

    if constexpr(std::is_same_v<RHIType, RHIBuffer>) {
      return create<RHIBuffer>(desc);
    }
    if constexpr(std::is_same_v<RHIType, Texture2>) {
      return create<Texture2>(desc);
    }
    
    BAD_CODE_PATH();
  }
  void free(RHIResource::sptr_t res, const RenderGraphResourceDesc& desc) {
	  mAvailableCache.emplace_back(Cache{desc, res});
  }

};

ResourcePool gResourcePool;

uint RenderGraphResourceSet::ResourceNode::sNextUsableId = 0;

RenderGraphResourceSet::ResourceCache* RenderGraphResourceSet::allocResouceCache(uint handleId) {
  ResourceCache* cache = new ResourceCache();

  mResourceMap[handleId] = cache;

  return cache;
}

void RenderGraphResourceSet::freeResourceCache(ResourceCache* res) {
  uint handleId;

  bool find = false;
  for(auto& [id, cache]: mResourceMap) {
    if(cache == res) {
      handleId = id;
      cache = nullptr;
      find = true;
    }
  }

  EXPECTS(find);
  delete res;

  mResourceMap.erase(handleId);
}

const Texture2::sptr_t& RenderGraphResourceSet::backBuffer() const {
  if(mCustomBackBuffer == nullptr) {
    return RHIDevice::get()->backBuffer();
  } else {
    return mCustomBackBuffer;
  }
}

void RenderGraphResourceSet::clearResources() {
  for(auto& [_, cache]: mResourceMap) {
    if(!cache->isStatic && cache->ready()) {
      eResolvePolicy policy = mResourceGraph.node(*cache)->resolvePolicy;
      if(policy == RESOLVE_FROM_CREATE || policy == RESOLVE_FROM_PARENT_CREATE) {
        RHIResource::sptr_t res = get<RHIResource>(*cache->handle);
        gResourcePool.free(res, cache->currentResDesc);
      }
      cache->res.reset();
    }
  }
}

RenderResourceHandle& RenderGraphResourceSet::extend(const RenderResourceHandle& source, const std::string& name, const RenderGraphResourceDesc& desc) {

  auto iter = mResourceMap.find(source.id);

  // the source is managed by the graph
  EXPECTS(iter != mResourceMap.end());

  auto* extendHandle = new RenderResourceHandle();

  *extendHandle = source.extend();
  extendHandle->desc = desc;

  ResourceCache* cache = allocResouceCache(extendHandle->id);

  cache->handle.reset(extendHandle);

  mResourceGraph.add(cache);
  mResourceGraph.connect(cache->nodeId, iter->second->nodeId, false);
  cache->handle->parentId = iter->second->handle->id;

  extendHandle->name = name;
  return *extendHandle;
}

RenderResourceHandle& RenderGraphResourceSet::extend(const std::string& source, const std::string& name, const RenderGraphResourceDesc& desc) {
  RenderResourceHandle* handle = find(source)->cast();
  return extend(*handle, name, desc);
}

const RenderGraphResourceHandle* RenderGraphResourceSet::find(std::string_view name) const {
  for(auto& [_, cache]: mResourceMap) {
    if(cache->handle->name == name) {
      return cache->handle.get();
    }
  }

  BAD_CODE_PATH();
}

RenderGraphResourceHandle* RenderGraphResourceSet::find(std::string_view name) {
  for(auto& [_, cache]: mResourceMap) {
    if(cache->handle->name == name) {
      return cache->handle.get();
    }
  }

  BAD_CODE_PATH();
}

void RenderGraphResourceSet::bind(std::string_view from, std::string_view to) {

  std::optional<ResourceCache*> fromCache, toCache;
  
  for(auto& [_, cache]: mResourceMap) {

    if(cache->handle->name == from) {
      fromCache = cache;
    }
    if(cache->handle->name == to) {
      toCache = cache;
    }

    if(fromCache.has_value() && toCache.has_value()) break;

  }

  mResourceGraph.connect((*fromCache)->nodeId, (*toCache)->nodeId, true);
  (*fromCache)->handle->parentId = (*toCache)->handle->id;
}

void RenderGraphResourceSet::setBackbuffer(const Texture2::sptr_t& backbuffer) {
  mCustomBackBuffer = backbuffer;
}

RenderResourceHandle& RenderGraphResourceSet::declare(std::string name, const RenderGraphResourceDesc& desc, const unique& type) {

  auto* handle = 
    new RenderResourceHandle(type, RenderGraphResourceHandle::OWN_RESOURCESET);
  ResourceCache* cache = allocResouceCache(handle->id);
  handle->desc = desc;

  cache->handle.reset(handle);
  mResourceGraph.add(cache);

  handle->name = name;

  return *handle;
}

RenderResourceHandle& RenderGraphResourceSet::declare(std::string name, const unique& type, RHIResource::Type rhiType) {

  auto* handle = 
    new RenderResourceHandle(type, RenderGraphResourceHandle::OWN_RESOURCESET);
  ResourceCache* cache = allocResouceCache(handle->id);

  cache->handle.reset(handle);
  mResourceGraph.add(cache);

  handle->name = name;
  handle->desc.type = rhiType;

  return *handle;
}

void RenderGraphResourceSet::ResourceNode::addIncoming(ResourceNode* node) {

  auto re = std::find(mIncomingNodes.begin(), mIncomingNodes.end(), node);

  EXPECTS(re == mIncomingNodes.end());
  EXPECTS(node != this);

  mIncomingNodes.emplace_back(node);
}

void RenderGraphResourceSet::ResourceNode::setOutgoing(ResourceNode* node) {
  EXPECTS(node != this);
  EXPECTS(mOutgoingNode == nullptr);

  mOutgoingNode = node;
}

void RenderGraphResourceSet::ResourceGraph::add(ResourceCache* content) {

  auto result = mNodes.emplace(content->nodeId, ResourceNode{content});

}

void RenderGraphResourceSet::ResourceGraph::connect(uint from, uint to, bool isReference) {

  ResourceNode* fromNode = node(from);
  ResourceNode* toNode = node(to);

  EXPECTS(fromNode != nullptr);
  EXPECTS(toNode != nullptr);

  fromNode->setOutgoing(toNode);
  toNode->addIncoming(fromNode);

  fromNode->resolvePolicy = isReference ? RESOLVE_FROM_PARENT_REFERENCE : RESOLVE_FROM_PARENT_CREATE;

}

RenderGraphResourceSet::ResourceNode* RenderGraphResourceSet::ResourceGraph::node(uint id) {
  auto iter = mNodes.find(id);

  return iter == mNodes.end() ? nullptr : &iter->second;
}

const RenderGraphResourceSet::ResourceNode* RenderGraphResourceSet::ResourceGraph::node(uint id) const {
  auto iter = mNodes.find(id);

  return iter == mNodes.end() ? nullptr : &iter->second;
}


void RenderGraphResourceSet::resolveResource(ResourceCache& target) const {

  // already resolved return
  if(target.ready()) return;

  const ResourceNode* node = mResourceGraph.node(target);
  RenderResourceHandle& handle = *target.handle->cast();

  
  // ------------- create ---------------
  if(node->resolvePolicy == RESOLVE_FROM_CREATE) {
    auto& desc = handle.desc;
    if(desc.type == RHIResource::Type::Buffer) {
      auto res = gResourcePool.acquire<RHIBuffer>(desc);
      setName(*res, make_wstring(handle.name).c_str());
      target.currentResDesc = desc;
      target.res = res;
      return;
    }

    if(desc.type == RHIResource::Type::Texture2D) {
      auto res = gResourcePool.acquire<Texture2>(desc);
      target.currentResDesc = desc;
      setName(*res, make_wstring(handle.name).c_str());
      target.res = res;
      return;
    }

    BAD_CODE_PATH();
  }

  // ------------- from parent ---------------
  {
    
    // not depends to any resource, the resource should already have been resolved in this case
    EXPECTS(target.handle->parentId != RenderGraphResourceHandle::INVALID_HANDLE_ID);

    // resolve parent first
    ResourceCache& parent = *mResourceMap.at(target.handle->parentId);
    resolveResource(parent);

    target.isStatic = false;

    if(node->resolvePolicy == RESOLVE_FROM_PARENT_REFERENCE) {
      target.res = parent.res;
      return;
    }

    EXPECTS(node->resolvePolicy == RESOLVE_FROM_PARENT_CREATE);

    RenderGraphResourceDesc desc;
    desc = handle.desc;
    if(desc.type == RHIResource::Type::Buffer) {
      RHIBuffer::sptr_t parentRes = get<RHIBuffer>(*parent.handle);


      desc.bindingFlags = desc.bindingFlags.value_or(parentRes->flags());
      desc.buffer.size = desc.buffer.size.value_or(parentRes->size());
      desc.buffer.cpuAccess = desc.buffer.cpuAccess.value_or(parentRes->cpuAccess());

      auto res = gResourcePool.acquire<RHIBuffer>(desc);
      target.currentResDesc = desc;
      setName(*res, make_wstring(handle.name).c_str());
      target.res = res;
     return;
    }

    if(desc.type == RHIResource::Type::Texture2D) {
      Texture2::sptr_t parentRes = get<Texture2>(*parent.handle);

      desc.bindingFlags = desc.bindingFlags.value_or(parentRes->flags());
      desc.texture2.size = desc.texture2.size.value_or(uvec2{parentRes->width(), parentRes->height()});
      desc.texture2.format = desc.texture2.format.value_or(parentRes->format());

      auto res = gResourcePool.acquire<Texture2>(desc);
      target.currentResDesc = desc;
      setName(*res, make_wstring(handle.name).c_str());
      target.res = res;
      return;
    }
  }

  // 
}

RenderGraphResourceHandle& RenderGraphResourceSet::declare(std::string name, const unique& type) {
  auto* handle = 
    new RenderGraphResourceHandle();
  ResourceCache* cache = allocResouceCache(handle->id);

  cache->handle.reset(handle);
  mResourceGraph.add(cache);

  handle->type = &type;
  handle->name = name;

  return *handle;
}

#include "RenderResourceHandle.hpp"


uint RenderGraphResourceHandle::sNextId = 0;

bool RenderGraphResourceDesc::Texture2Desc::operator==(const Texture2Desc& rhs) const {
  return format == rhs.format && size == rhs.size;
}

bool RenderGraphResourceDesc::BufferDesc::operator==(const BufferDesc& rhs) const {
  return size == rhs.size && cpuAccess == rhs.cpuAccess;
}

bool RenderGraphResourceDesc::operator==(const RenderGraphResourceDesc& rhs) const {
  if(type != rhs.type) return false;
  if(bindingFlags != rhs.bindingFlags) return false;
  switch(type) { 
    case RHIResource::Type::Buffer:
      return buffer == rhs.buffer;
    break;
    case RHIResource::Type::Texture2D:
      return texture2 == rhs.texture2;
    break;
    case RHIResource::Type::Unknown: ;
    case RHIResource::Type::Texture1D: ;
    case RHIResource::Type::Texture3D: ;
    case RHIResource::Type::TextureCube: ;
    case RHIResource::Type::Texture2DMultisample: ;
    default: ;
    BAD_CODE_PATH();
  }
}

RenderResourceHandle::RenderResourceHandle(const unique& t, eOwnership owner): RenderGraphResourceHandle() {
  this->type  = &t;
  this->owner = owner;
  resourceType = HANDLE_RES_RHI;
}

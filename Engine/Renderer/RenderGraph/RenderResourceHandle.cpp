#include "RenderResourceHandle.hpp"


uint RenderGraphResourceHandle::sNextId = 0;

RenderResourceHandle::RenderResourceHandle(const unique& t, eOwnership owner): RenderGraphResourceHandle() {
  this->type  = &t;
  this->owner = owner;
  resourceType = HANDLE_RES_RHI;
}

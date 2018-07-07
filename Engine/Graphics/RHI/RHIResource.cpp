#include "RHI.hpp"
#include "RHIResource.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

RHIResource::~RHIResource() {
  if(RHIDevice::get())
    RHIDevice::get()->releaseResource(mRhiHandle);
}


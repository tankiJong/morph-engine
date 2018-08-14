#include "RHI.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "RHIResource.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

RHIResource::~RHIResource() {
  if(RHIDevice::get())
    RHIDevice::get()->releaseResource(mRhiHandle);
}


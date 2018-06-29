#include "RHI.hpp"
#include "RHIResource.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

RHIResource::~RHIResource() {
  RHIDevice::get()->releaseResource(mRhiHandle);
}

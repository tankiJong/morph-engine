#include "PipelineState.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

GraphicsState::~GraphicsState() {
  RHIDevice::get()->releaseResource(mRhiHandle);
}

GraphicsState::sptr_t GraphicsState::create(const Desc& desc) {
  sptr_t state = sptr_t(new GraphicsState(desc));

  if(state->rhiInit() == false) {
    DEBUGBREAK;
    state = nullptr;
  }

  return state;
}


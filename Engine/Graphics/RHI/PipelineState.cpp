#include "PipelineState.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

PipelineState::~PipelineState() {
  RHIDevice::get()->releaseResource(mRhiHandle);
}

PipelineState::sptr_t PipelineState::create(const Desc& desc) {
  sptr_t state = sptr_t(new PipelineState(desc));

  if(state->rhiInit() == false) {
    DEBUGBREAK;
    state = nullptr;
  }

  return state;
}


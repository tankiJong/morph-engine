#include "PipelineState.hpp"

PipelineState::sptr_t PipelineState::create(const Desc& desc) {
  sptr_t state = sptr_t(new PipelineState(desc));

  if(state->rhiInit() == false) {
    DEBUGBREAK;
    state = nullptr;
  }

  return state;
}


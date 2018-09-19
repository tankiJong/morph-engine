#include "PipelineState.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/Program/Program.hpp"

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

ComputeState::Desc& ComputeState::Desc::setProgram(S<const Program> prog) {
  mProg = prog;
  auto rs = prog->rootSignature();

  if(rs) {
    mRootSignature = rs;
  }
  return *this;
}
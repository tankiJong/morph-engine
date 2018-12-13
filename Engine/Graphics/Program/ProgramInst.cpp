#include "ProgramInst.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/RootSignature.hpp"
#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

D3D12_DESCRIPTOR_RANGE_TYPE asDx12RangeType(const RootSignature::desc_type_t type);

void ProgramInst::setCbv(const ConstantBufferView& cbv, uint registerIndex, uint registerSpace) {
  mProgramData.setCbv(cbv, registerIndex, registerSpace);
}

void ProgramInst::setSrv(const ShaderResourceView& srv, uint registerIndex, uint registerSpace) {
  mProgramData.setSrv(srv, registerIndex, registerSpace);
}

void ProgramInst::setUav(const UnorderedAccessView& uav, uint registerIndex, uint registerSpace) {
  mProgramData.setUav(uav, registerIndex, registerSpace);
}

ProgramInst::ProgramInst(const S<const Program>& program) {
  mProg = program;

  auto layouts = mProg->descriptorLayout();

  mProgramData.init(layouts);
}

bool GraphicsProgramInst::apply(RHIContext& ctx, bool bindRootSignature) {
  return mProgramData.bindForGraphics(ctx, *mProg->rootSignature(), bindRootSignature);
}

S<GraphicsProgramInst> GraphicsProgramInst::create(const S<const Program>& program) {
  return S<GraphicsProgramInst>(new GraphicsProgramInst(program));
}

bool ComputeProgramInst::apply(RHIContext& ctx, bool bindRootSignature) {
  return mProgramData.bindForCompute(ctx, *mProg->rootSignature(), bindRootSignature);
}

S<ComputeProgramInst> ComputeProgramInst::create(const S<Program>& program) {
  return S<ComputeProgramInst>(new ComputeProgramInst(program));
}

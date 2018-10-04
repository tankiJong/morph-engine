#include "ProgramIns.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/RootSignature.hpp"
#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

D3D12_DESCRIPTOR_RANGE_TYPE asDx12RangeType(const RootSignature::desc_type_t type);


void ProgramIns::setCbv(const ConstantBufferView& cbv, uint registerIndex, uint registerSpace) {

  loc_t loc = locateBindPoint(cbv.info().type, registerIndex, registerSpace);
  mDescriptorSets[loc.descriptorSetIndex]->setCbv(loc.rangeIndex, loc.zeroOffset, cbv);
}

void ProgramIns::setSrv(const ShaderResourceView& srv, uint registerIndex, uint registerSpace) {

  loc_t loc = locateBindPoint(srv.info().type, registerIndex, registerSpace);
  mDescriptorSets[loc.descriptorSetIndex]->setSrv(loc.rangeIndex, loc.zeroOffset, srv);
}

void ProgramIns::setUav(const UnorderedAccessView& uav, uint registerIndex, uint registerSpace) {
  
  loc_t loc = locateBindPoint(uav.info().type, registerIndex, registerSpace);
  mDescriptorSets[loc.descriptorSetIndex]->setUav(loc.rangeIndex, loc.zeroOffset, uav);
}

ProgramIns::ProgramIns(const S<Program>& program) {
  mProg = program;

  auto layouts = mProg->descriptorLayout();

  mDescriptorSets.reserve(layouts.size());

  auto& device = RHIDevice::get();

  for(auto& layout: layouts) {
    mDescriptorSets.emplace_back(DescriptorSet::create(device->gpuDescriptorPool(), layout));
  }
}

ProgramIns::loc_t ProgramIns::locateBindPoint(DescriptorPool::Type type, uint registerIndex, uint registerSpace) const {

  loc_t loc { INVALID_SET_INDEX, 0, 0};

  auto dx12Type = asDx12RangeType(type);
  for(uint i = 0; i < mDescriptorSets.size(); i++) {
    const S<DescriptorSet>& set = mDescriptorSets[i];

    for(size_t k = 0; k < set->rangeCount(); k++) {
      auto& range = set->range(k);

      if (dx12Type != asDx12RangeType(range.type)) continue;
      if (registerSpace != range.registerSpace) continue;
      if (registerIndex < range.baseRegisterIndex || (registerIndex > range.baseRegisterIndex + range.descCount)) continue;

      loc.descriptorSetIndex = i;
      loc.rangeIndex = k;
      loc.zeroOffset = registerIndex - range.baseRegisterIndex;

      return loc;
    }
  }
  ENSURES(loc.descriptorSetIndex != INVALID_SET_INDEX);

  return loc;
}

void GraphicsProgramIns::apply(RHIContext& ctx, bool bindRootSignature) {
  if(bindRootSignature) {
    ctx.setGraphicsRootSignature(*mProg->rootSignature());
  }

  for(size_t i = 0; i < mDescriptorSets.size(); i++) {
    mDescriptorSets[i]->bindForGraphics(ctx, *mProg->rootSignature(), i);
  }
}

S<GraphicsProgramIns> GraphicsProgramIns::create(const S<Program>& program) {
  return S<GraphicsProgramIns>(new GraphicsProgramIns(program));
}

void ComputeProgramIns::apply(RHIContext& ctx, bool bindRootSignature) {
  if (bindRootSignature) {
    ctx.setComputeRootSignature(*mProg->rootSignature());
  }

  for (size_t i = 0; i < mDescriptorSets.size(); i++) {
    mDescriptorSets[i]->bindForCompute(ctx, *mProg->rootSignature(), i);
  }
}

S<ComputeProgramIns> ComputeProgramIns::create(const S<Program>& program) {
  return S<ComputeProgramIns>(new ComputeProgramIns(program));
}

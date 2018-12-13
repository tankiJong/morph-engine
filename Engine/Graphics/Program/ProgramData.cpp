#include "ProgramData.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12RootSignature.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"

void ProgramData::init(RootSignature::sptr_t rootsig) {
  auto sets = rootsig->desc().sets();
  init(sets);
}

void ProgramData::init(span<const DescriptorSet::Layout> sets) {
  
  mLayouts.resize(sets.size());

  std::copy(sets.begin(), sets.end(), mLayouts.begin());

  mParamDatas.resize(sets.size());

  for(std::ptrdiff_t i = 0; i < sets.size(); i++) {
    mParamDatas[i].data = ParamData::create(sets[i]);
    mParamDatas[i].bind = true;
  }
}

bool ProgramData::setSrv(const ShaderResourceView& srv, uint registerIndex, uint registerSpace) {
  ParamData::BindPoint bp = locateBindPoint(DescriptorPool::Type::TextureSrv, registerIndex, registerSpace);
  mParamDatas[bp.descriptorSetIndex].bind = true;
  return mParamDatas[bp.descriptorSetIndex].data->setSrv(srv, bp);
}

bool ProgramData::setCbv(const ConstantBufferView& cbv, uint registerIndex, uint registerSpace) {
  ParamData::BindPoint bp = locateBindPoint(DescriptorPool::Type::Cbv, registerIndex, registerSpace);
  mParamDatas[bp.descriptorSetIndex].bind = true;
  return mParamDatas[bp.descriptorSetIndex].data->setCbv(cbv, bp);
}

bool ProgramData::setUav(const UnorderedAccessView& uav, uint registerIndex, uint registerSpace) {
  ParamData::BindPoint bp = locateBindPoint(DescriptorPool::Type::TextureUav, registerIndex, registerSpace);
  mParamDatas[bp.descriptorSetIndex].bind = true;
  return mParamDatas[bp.descriptorSetIndex].data->setUav(uav, bp);
}

bool ProgramData::bindForGraphics(RHIContext& ctx, const RootSignature& rootSig, bool bindRootSignature) {
  if(bindRootSignature) {
    ctx.setGraphicsRootSignature(rootSig);
  }

  bool result = true;
  for(size_t i = 0; i < mParamDatas.size(); i++) {
    auto& paramData = mParamDatas[i];
    result = result && paramData.data->finalize();
    if(paramData.bind) {
      paramData.data->descriptorSet()->bindForGraphics(ctx, rootSig, i);
    }
  }

  return result;
}

bool ProgramData::bindForCompute(RHIContext& ctx, const RootSignature& rootSig, bool bindRootSignature) {
  if(bindRootSignature) {
    ctx.setComputeRootSignature(rootSig);
  }

  bool result = true;
  for(size_t i = 0; i < mParamDatas.size(); i++) {
    auto& paramData = mParamDatas[i];
    result = result && paramData.data->finalize();
    if(paramData.bind) {
      paramData.data->descriptorSet()->bindForCompute(ctx, rootSig, i);
    }
  }

  return result;
}

ParamData::BindPoint ProgramData::locateBindPoint(DescriptorPool::Type type, uint registerIndex, uint registerSpace) const {
  
  ParamData::BindPoint loc { INVALID_SET_INDEX, 0, 0 };

  auto dx12Type = asDx12RangeType(type);
  auto& sets = mLayouts;

  for(uint i = 0; i < sets.size(); i++) {
    auto& set = sets[i];

    for(size_t k = 0; k < set.rangeCount(); k++) {
      auto& range = set.range((uint)k);

      if (dx12Type != asDx12RangeType(range.type)) continue;
      if (registerSpace != range.registerSpace) continue;
      if (   registerIndex < range.baseRegisterIndex 
         || (registerIndex >= range.baseRegisterIndex + range.descCount)) continue;

      loc.descriptorSetIndex = i;
      loc.rangeIndex = k;
      loc.zeroOffset = registerIndex - range.baseRegisterIndex;

      return loc;
    }
  }
  ENSURES(loc.descriptorSetIndex != INVALID_SET_INDEX);

  return loc;
}




#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include <vector>
#include "Engine/Graphics/Program/ParamData.hpp"
#include "Engine/Graphics/RHI/RootSignature.hpp"

/*
 the structure is very closely mapped to Root Signature:

 ParamData <---- "DescriptorTable(SRV(t10, numDescriptors = 6), ...)," \  ┐
                                                                          ├ Program Data
 ParamData <---- "DescriptorTable(UAV(u0, numDescriptors = 3), ...)," \   ┘
                                 └────────────┬─────────────┘
                                          BlockData
 */
class ProgramData {
public:
  static constexpr uint INVALID_SET_INDEX = UINT_MAX;
  
  void init(RootSignature::sptr_t rootsig);
  void init(span<const DescriptorSet::Layout> set);
  
  bool setSrv(const ShaderResourceView& srv, uint registerIndex, uint registerSpace = 0);
  bool setCbv(const ConstantBufferView& cbv, uint registerIndex, uint registerSpace = 0);
  bool setUav(const UnorderedAccessView& uav, uint registerIndex, uint registerSpace = 0);

  bool bindForGraphics(RHIContext& ctx, const RootSignature& rootSig, bool bindRootSignature);
  bool bindForCompute(RHIContext& ctx, const RootSignature& rootSig, bool bindRootSignature);
protected:

  struct FlagedParamData {
    ParamData::sptr_t data;
    bool bind = false;
  };

  ParamData::BindPoint locateBindPoint(DescriptorPool::Type type, uint registerIndex, uint registerSpace) const;

  std::vector<DescriptorSet::Layout> mLayouts;
  std::vector<FlagedParamData> mParamDatas;
};

#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"

/*
 the structure is very closely mapped to Root Signature:

 ParamData <---- "DescriptorTable(SRV(t10, numDescriptors = 6), ...)," \  ┐
                                                                          ├ Program Data
 ParamData <---- "DescriptorTable(UAV(u0, numDescriptors = 3), ...)," \   ┘
                                 └────────────┬─────────────┘
                                          BlockData
 */


class ParamData {
public:
  using sptr_t = S<ParamData>;
  using csptr_t = S<const ParamData>;

  // TODO: potentially should live in the reflection
  struct BindPoint {
    uint descriptorSetIndex;
    size_t rangeIndex;
    uint zeroOffset;
  };

  struct ResourceRef {
    RHIResource::scptr_t res;
    DescriptorSet::Type type;

    union {
      const ConstantBufferView* cbv;
      const ShaderResourceView* srv;
      const UnorderedAccessView* uav;
    };

    // size_t requiredSize = 0;
  };

  bool setSrv(const ShaderResourceView& srv, const BindPoint& bp);
  bool setCbv(const ConstantBufferView& cbv, const BindPoint& bp);
  bool setUav(const UnorderedAccessView& uav,const BindPoint& bp);

  bool finalize();
  const DescriptorSet::sptr_t& descriptorSet() const { return mDescriptorSet; }
  
  static sptr_t create(const DescriptorSet::Layout& layout);

protected:


  using BlockData = std::vector<ResourceRef>;

  std::vector<BlockData> mBindedResources;
  DescriptorSet::sptr_t mDescriptorSet;
  bool mDirty = true;
};

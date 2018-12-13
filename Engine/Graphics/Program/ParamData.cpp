#include "ParamData.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/Log.hpp"

bool ParamData::setSrv(const ShaderResourceView& srv, const BindPoint& bp) {
  
  auto& res = mBindedResources[bp.rangeIndex][bp.zeroOffset];

  res.res = srv.res().lock();
  ASSERT_OR_RETURN(res.res != nullptr, false);

  res.type = DescriptorPool::Type::TextureSrv;
  // res.requiredSize = 
  res.srv = &srv;

  mDirty = true;
  return true;
}

bool ParamData::setCbv(const ConstantBufferView& cbv, const BindPoint& bp) {
  
  auto& res = mBindedResources[bp.rangeIndex][bp.zeroOffset];

  res.res = cbv.res().lock();
  ASSERT_OR_RETURN(res.res != nullptr, false);

  res.type = DescriptorPool::Type::Cbv;
  // res.requiredSize = 
  res.cbv = &cbv;

  mDirty = true;
  return true;
}

bool ParamData::setUav(const UnorderedAccessView& uav, const BindPoint& bp) {
  auto& res = mBindedResources[bp.rangeIndex][bp.zeroOffset];

  res.res = uav.res().lock();
  ASSERT_OR_RETURN(res.res != nullptr, false);

  res.type = DescriptorPool::Type::Cbv;
  // res.requiredSize = 
  res.uav = &uav;

  mDirty = true;
  return true;
}

bool ParamData::finalize() {
  if(!mDirty) return true;

  for(size_t rangeIndex = 0; rangeIndex < mBindedResources.size(); rangeIndex++) {
    BlockData& block = mBindedResources[rangeIndex];

    for(size_t offset = 0; offset < block.size(); offset++) {
      ResourceRef& res = block[offset];

      if(res.res == nullptr) {
        Log::logf("descriptor[%u][%u] is empty, skip", rangeIndex, offset);
        continue;
      }
      switch(res.type) {
        case DescriptorSet::Type::Cbv: {
          EXPECTS(res.cbv);
          mDescriptorSet->setCbv(rangeIndex, offset, *res.cbv);
        }
        break;
        case DescriptorSet::Type::Sampler:
          UNIMPLEMENTED();
        break;
        case DescriptorSet::Type::StructuredBufferSrv:
        case DescriptorSet::Type::TypedBufferSrv:
        case DescriptorSet::Type::TextureSrv:
          EXPECTS(res.srv);
          mDescriptorSet->setSrv(rangeIndex, offset, *res.srv);
        break;
        case DescriptorSet::Type::StructuredBufferUav:
        case DescriptorSet::Type::TypedBufferUav:
        case DescriptorSet::Type::TextureUav:
          EXPECTS(res.uav);
          mDescriptorSet->setUav(rangeIndex, offset, *res.uav);
        break;

        default:
        BAD_CODE_PATH();
        return false;
      }
    }
  }

  return true;
}

ParamData::sptr_t ParamData::create(const DescriptorSet::Layout& layout) {

  auto* paramData = new ParamData();
  paramData->mBindedResources.resize(layout.rangeCount());

  for(uint i = 0; i < layout.rangeCount(); i++) {
    paramData->mBindedResources[i].resize(layout.range(i).descCount);
  }

  paramData->mDescriptorSet = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

  return sptr_t{ paramData };
}

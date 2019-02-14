#include "ParamData.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/Log.hpp"

bool ParamData::setSrv(const ShaderResourceView& srv, const BindPoint& bp) {
  auto& res = mBindedResources[bp.rangeIndex][bp.zeroOffset];

  EXPECTS(res.type == DescriptorPool::Type::TypedBufferSrv ||
          res.type == DescriptorPool::Type::TextureSrv ||
          res.type == DescriptorPool::Type::StructuredBufferSrv);

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

  EXPECTS(res.type == DescriptorPool::Type::Cbv);

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

  EXPECTS(res.type == DescriptorPool::Type::TypedBufferUav ||
          res.type == DescriptorPool::Type::TextureUav ||
          res.type == DescriptorPool::Type::StructuredBufferUav);

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

  for(uint rangeIndex = 0; rangeIndex < (uint)mBindedResources.size(); rangeIndex++) {
    BlockData& block = mBindedResources[rangeIndex];

    for(uint offset = 0; offset < (uint)block.size(); offset++) {
      ResourceRef& res = block[offset];

      if(res.res == nullptr) {
        Log::logf("descriptor[%u][%u] is empty, try to bind null view", rangeIndex, offset);
      }

      switch(res.type) {
        case DescriptorSet::Type::Cbv: {
          if(res.res == nullptr) {
            res.cbv = ConstantBufferView::nullView().get();
          }
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
          if(res.res == nullptr) {
            res.srv = ShaderResourceView::nullView().get();
          }
          EXPECTS(res.srv);
          mDescriptorSet->setSrv(rangeIndex, offset, *res.srv);
        break;
        case DescriptorSet::Type::StructuredBufferUav:
        case DescriptorSet::Type::TypedBufferUav:
        case DescriptorSet::Type::TextureUav:
          if(res.res == nullptr) {
            res.uav = UnorderedAccessView::nullView().get();
          }
          EXPECTS(res.uav);
          mDescriptorSet->setUav(rangeIndex, offset, *res.uav);
        break;

        default:
        BAD_CODE_PATH();
        return false;
      }
    }
  }

  mDirty = false;
  return true;
}

ParamData::sptr_t ParamData::create(const DescriptorSet::Layout& layout) {

  auto* paramData = new ParamData();
  paramData->mBindedResources.resize(layout.rangeCount());

  for(uint i = 0; i < layout.rangeCount(); i++) {\
    auto& range = layout.range(i);
    paramData->mBindedResources[i].resize(range.descCount);

    for(uint j = 0; j < range.descCount; j++) {
      ResourceRef& ref = paramData->mBindedResources[i][j];
      ref.res = nullptr;
      ref.type = range.type;
      ref.cbv = nullptr;
    }
  }

  paramData->mDescriptorSet = DescriptorSet::create(RHIDevice::get()->gpuDescriptorPool(), layout);

  return sptr_t{ paramData };
}

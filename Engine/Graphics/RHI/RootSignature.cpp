#include "RootSignature.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

u64 RootSignature::sObjectCoount = 0;
RootSignature::sptr_t RootSignature::sEmptySignature;

RootSignature::Desc& RootSignature::Desc::addDescriptorSet(const DescriptorSet::Layout& setLayout) {
  EXPECTS(setLayout.rangeCount());
  mSets.push_back(setLayout);

  return *this;
}

RootSignature::RootSignature(const Desc& desc): mDesc(desc) {
  sObjectCoount++;
}


RootSignature::sptr_t RootSignature::create(const Desc& desc) {
  bool empty = desc.mSets.empty();
  if (empty && sEmptySignature) return sEmptySignature;

  sptr_t sig = sptr_t(new RootSignature(desc));
  if (sig->rhiInit() == false) {
    sig = nullptr;
  }

  if (empty) sEmptySignature = sig;

  return sig;
}

RootSignature::sptr_t RootSignature::create(const Blob& data) {
  sptr_t sig = sptr_t(new RootSignature());
  sig->mFromBlob = true;

  sig->initHandle(data);

  return sig;
}

S<const RootSignature> RootSignature::emptyRootSignature() {
  if(!sEmptySignature) {
    Desc desc;
    sptr_t sig = sptr_t(new RootSignature(desc));
    if (sig->rhiInit() == false) {
      BAD_CODE_PATH();
    }

    sEmptySignature = sig;
  }

  return sEmptySignature;
}

bool RootSignature::operator==(const RootSignature& rhs) const {
  return mRhiHandle.Get() == rhs.mRhiHandle.Get();
}

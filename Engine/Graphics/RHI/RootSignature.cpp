#include "RootSignature.hpp"

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
  bool empty = desc.mSets.size() == 0;
  if (empty && sEmptySignature) return sEmptySignature;

  sptr_t sig = sptr_t(new RootSignature(desc));
  if (sig->rhiInit() == false) {
    sig == nullptr;
  }

  if (empty) sEmptySignature = sig;

  return sig;
}
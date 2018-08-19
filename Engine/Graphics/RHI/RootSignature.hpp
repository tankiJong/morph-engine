#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"

class Blob;

class RootSignature {
public:
  using sptr_t = S<RootSignature>;
  using scptr_t = S<const RootSignature>;
  using rhi_handle_t = root_signature_handle_t;

  using desc_type_t = DescriptorSet::Type;
  using desc_set_layout_t = DescriptorSet::Layout;

  class Desc {
  public:
    Desc & addDescriptorSet(const desc_set_layout_t& setLayout);
  protected:
    friend class RootSignature;
    std::vector<desc_set_layout_t> mSets;
  };

  rhi_handle_t handle() const { return mRhiHandle; };

  static sptr_t create(const Desc& desc);
#ifdef MORPH_D3D12
  static sptr_t create(const Blob& data);
#endif
  static S<const RootSignature> emptyRootSignature();

  bool operator==(const RootSignature& rhs) const;
protected:
  bool rhiInit();
#ifdef MORPH_D3D12
  virtual void initHandle(ID3DBlobPtr sigBlob);
  virtual void initHandle(const Blob& sigBlob);
#endif
  RootSignature(const Desc& desc);
  RootSignature() = default;

  Desc mDesc;
  static sptr_t sEmptySignature;
  static u64 sObjectCoount;
  uint mSizeInBytes;
  rhi_handle_t mRhiHandle;
  std::vector<uint> mElementByteOffset;
  bool mFromBlob = false;
};

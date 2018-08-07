#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHI.hpp"

class RHIResource;
class RHIBuffer;
class RHITexture;
class Texture2;

struct ResourceViewInfo {
  uint mostDetailedMip;
  uint mipCount;
  uint firstArraySlice;
  uint arraySize;
  ResourceViewInfo(
    uint mostDetailedMip, uint mipCount, 
    uint firstArraySlice, uint arraySize)
  : mostDetailedMip(mostDetailedMip), mipCount(mipCount)
  , firstArraySlice(firstArraySlice), arraySize(arraySize) {}

  inline bool operator==(const ResourceViewInfo& rhs) const {
    return
      (firstArraySlice == rhs.firstArraySlice) &&
      (arraySize == rhs.arraySize) &&
      (mipCount == rhs.mipCount) &&
      (mostDetailedMip == rhs.mostDetailedMip);
  }
};

template<typename handle_t>
class ResourceView {
public:
  using rhi_handle_t = handle_t;
  static const uint MAX_POSSIBLE = uint(-1);
  virtual ~ResourceView() {};

  ResourceView(
    W<RHIResource> res, rhi_handle_t handle, 
    uint mostDetailedMip, uint mipCount, uint firstArraySlice, uint arraySize)
    : mRhiHandle(handle), mResource(res)
    , mViewInfo(mostDetailedMip, mipCount, firstArraySlice, arraySize) {}

  W<RHIResource> res() const { return mResource; }
  const rhi_handle_t& handle() const { return mRhiHandle; }
protected:
  rhi_handle_t mRhiHandle;
  ResourceViewInfo mViewInfo;
  W<RHIResource> mResource;
};

class ShaderResourceView: public ResourceView<srv_handle_t> {
public:
  using sptr_t = S<ShaderResourceView>;
  using scptr_t = S<const ShaderResourceView>;

  static sptr_t create(W<RHITexture> res,
                       uint mostDetailedMip = 0, uint mipCount = MAX_POSSIBLE, uint firstArraySlice = 0, uint arraySize = MAX_POSSIBLE);
  // static sptr_t create(W<RHIBuffer> res,
  //                      uint mostDetailedMip = 0, uint mipCount = MAX_POSSIBLE, uint firstArraySlice = 0, uint arraySize = MAX_POSSIBLE);
  static sptr_t nullView();
protected:
  ShaderResourceView(W<RHIResource> res, rhi_handle_t handle, 
                     uint mostDetailedMip, uint mipCount, uint firstArraySlice, uint arraySize)
    : ResourceView(res, handle, mostDetailedMip, mipCount, firstArraySlice, arraySize) {}
  static sptr_t sNullView;
};

class ConstantBufferView: public ResourceView<cbv_handle_t> {
public:
  using sptr_t = S<ConstantBufferView>;
  using scptr_t = S<const ConstantBufferView>;

  static sptr_t create(W<RHIBuffer> res);
  static sptr_t nullView();

protected:
  ConstantBufferView(W<RHIResource> res, rhi_handle_t handle)
    : ResourceView(res, handle, 0, 1, 0, 1) {}

  static sptr_t sNullView;
};

class RenderTargetView: public ResourceView<rtv_handle_t> {
public:
  using sptr_t = S<RenderTargetView>;
  using scptr_t = S<const RenderTargetView>;

  static sptr_t create(W<RHITexture> res, uint mipLevel = 0, uint firstArraySlice = 0, uint arraySize = MAX_POSSIBLE);
  static sptr_t nullView();

protected:
  RenderTargetView(W<RHIResource> res, rhi_handle_t handle, uint mipLevel, uint firstArraySlice, uint arraySize): ResourceView(res, handle, mipLevel, 1, firstArraySlice, arraySize) {}

  static sptr_t sNullView;
};

class DepthStencilView : public ResourceView<dsv_handle_t> {
public:
  using sptr_t = S<DepthStencilView>;
  using scptr_t = S<const DepthStencilView>;

  static sptr_t create(W<RHITexture> res, uint mipLevel = 0, uint firstArraySlice = 0, uint arraySize = MAX_POSSIBLE);
  static sptr_t nullView();

protected:
  DepthStencilView(W<RHIResource> res, rhi_handle_t handle, uint mipLevel, uint firstArraySlice, uint arraySize) : ResourceView(res, handle, mipLevel, 1, firstArraySlice, arraySize) {}

  static sptr_t sNullView;
};

class UnorderedAccessView : public ResourceView<uav_handle_t> {
public:
  using sptr_t = S<UnorderedAccessView>;
  using scptr_t = S<const UnorderedAccessView>;

  static sptr_t create(W<Texture2> res, uint mipLevel = 0);
  static sptr_t nullView();

protected:
  UnorderedAccessView(W<RHIResource> res, rhi_handle_t handle, uint mipLevel, uint firstArraySlice, uint arraySize) : ResourceView(res, handle, mipLevel, 1, firstArraySlice, arraySize) {}

  static sptr_t sNullView;
};
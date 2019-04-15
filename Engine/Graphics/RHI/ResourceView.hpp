#pragma once

#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/DescriptorPool.hpp"


class RHIResource;
class RHIBuffer;
class RHITexture;
class Texture2;
class TypedBuffer;

struct ResourceViewInfo {
  static const uint MAX_POSSIBLE = uint(-1);

  uint arraySize;
  uint firstArraySlice;
  uint mostDetailedMip;
  uint mipCount;
  DescriptorPool::Type type;

  ResourceViewInfo() = default;

  ResourceViewInfo(
    uint mostDetailedMip, uint mipCount, 
    uint firstArraySlice, uint arraySize, DescriptorPool::Type type)
  : mostDetailedMip(mostDetailedMip), mipCount(mipCount)
  , firstArraySlice(firstArraySlice), arraySize(arraySize), type(type) {}

  inline bool operator==(const ResourceViewInfo& rhs) const {
    return
      (firstArraySlice == rhs.firstArraySlice) &&
      (arraySize == rhs.arraySize) &&
      (mipCount == rhs.mipCount) &&
      (mostDetailedMip == rhs.mostDetailedMip);
  }
};


namespace std {
  template<>
  struct hash<ResourceViewInfo> {
    size_t operator()(const ResourceViewInfo& info) const noexcept {
      size_t 
      result = info.mostDetailedMip;

      result ^= info.mipCount;
      result <<= 1;

      result ^= info.firstArraySlice;
      result <<= 1;

      result ^= info.arraySize;
      result <<= 1;

      result ^= (size_t)info.type;

      return  result;
    }
  };
}

template<typename handle_t>
class ResourceView {
public:
  using rhi_handle_t = handle_t;
  virtual ~ResourceView() {};

  ResourceView(
    W<const RHIResource> res, rhi_handle_t handle, 
    uint mostDetailedMip, uint mipCount, uint firstArraySlice, uint arraySize, DescriptorPool::Type type)
    : mRhiHandle(handle), mResource(res)
    , mViewInfo(mostDetailedMip, mipCount, firstArraySlice, arraySize, type) {}

  W<const RHIResource> res() const { return mResource; }
  const rhi_handle_t& handle() const { return mRhiHandle; }
  const ResourceViewInfo& info() const { return mViewInfo; }

protected:
  rhi_handle_t mRhiHandle;
  ResourceViewInfo mViewInfo;
  W<const RHIResource> mResource;
};

class ShaderResourceView: public ResourceView<srv_handle_t> {
public:
  using sptr_t = S<ShaderResourceView>;
  using scptr_t = S<const ShaderResourceView>;

  static sptr_t create(W<const RHITexture> res,
                       uint mostDetailedMip = 0, uint mipCount = ResourceViewInfo::MAX_POSSIBLE, uint firstArraySlice = 0, uint arraySize = ResourceViewInfo::MAX_POSSIBLE);
  static sptr_t create(const TypedBuffer& res);
  static sptr_t nullView();
protected:
  ShaderResourceView(W<const RHIResource> res, DescriptorPool::Type type, rhi_handle_t handle,
                     uint mostDetailedMip, uint mipCount, uint firstArraySlice, uint arraySize)
    : ResourceView(res, handle, mostDetailedMip, mipCount, firstArraySlice, arraySize, type) {}
  static sptr_t sNullView;
};

class ConstantBufferView: public ResourceView<cbv_handle_t> {
public:
  using sptr_t = S<ConstantBufferView>;
  using scptr_t = S<const ConstantBufferView>;

  static sptr_t create(W<const RHIBuffer> res);
  static sptr_t nullView();

protected:
  ConstantBufferView(W<const RHIResource> res, rhi_handle_t handle)
    : ResourceView(res, handle, 0, 1, 0, 1, DescriptorPool::Type::Cbv) {}

  static sptr_t sNullView;
};

class RenderTargetView: public ResourceView<rtv_handle_t> {
public:
  using sptr_t = S<RenderTargetView>;
  using scptr_t = S<const RenderTargetView>;

  static sptr_t create(W<const RHITexture> res, uint mipLevel = 0, uint firstArraySlice = 0, uint arraySize = ResourceViewInfo::MAX_POSSIBLE);
  static sptr_t nullView();

protected:
  RenderTargetView(W<const RHIResource> res, DescriptorPool::Type type, rhi_handle_t handle, uint mipLevel, uint firstArraySlice, uint arraySize): ResourceView(res, handle, mipLevel, 1, firstArraySlice, arraySize, type) {}

  static sptr_t sNullView;
};

class DepthStencilView : public ResourceView<dsv_handle_t> {
public:
  using sptr_t = S<DepthStencilView>;
  using scptr_t = S<const DepthStencilView>;

  static sptr_t create(W<const RHITexture> res, uint mipLevel = 0, uint firstArraySlice = 0, uint arraySize = ResourceViewInfo::MAX_POSSIBLE);
  static sptr_t nullView();

protected:
  DepthStencilView(W<const RHIResource> res, DescriptorPool::Type type, rhi_handle_t handle, 
                   uint mipLevel, uint firstArraySlice, uint arraySize) : ResourceView(res, handle, mipLevel, 1, firstArraySlice, arraySize, type) {}

  static sptr_t sNullView;
};

class UnorderedAccessView : public ResourceView<uav_handle_t> {
public:
  using sptr_t = S<UnorderedAccessView>;
  using scptr_t = S<const UnorderedAccessView>;

  static sptr_t create(W<const RHITexture> res, uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t arraySize = ResourceViewInfo::MAX_POSSIBLE);
  static sptr_t create(W<const RHIBuffer> res);
  static sptr_t create(const TypedBuffer& res);
  static sptr_t nullView();

protected:
  UnorderedAccessView(W<const RHIResource> res, DescriptorPool::Type type, rhi_handle_t handle, 
                      uint mipLevel, uint firstArraySlice, uint arraySize) 
  : ResourceView(res, handle, mipLevel, 1, firstArraySlice, arraySize, type) {}

  static sptr_t sNullView;
};
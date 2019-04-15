#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "RHITexture.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"

ShaderResourceView* RHITexture::srv(uint mipLevel, uint mipCount) const {

  if(mipCount == ResourceViewInfo::MAX_POSSIBLE) {
    mipCount = mMipLevels - mipLevel;
  }
  ResourceViewInfo info(mipLevel, mipCount, 0, mDepthOrArraySize, DescriptorPool::Type::TextureSrv);

  auto kv = mSrvs.find(info);

  if (kv == mSrvs.end() && is_set(mBindingFlags, BindingFlag::ShaderResource)) {
    ShaderResourceView::sptr_t ptr = ShaderResourceView::create(
        shared_from_this(), info.mostDetailedMip, 
        info.mipCount, info.firstArraySlice, info.arraySize);

    ENSURES(ptr->info() == info);

    mSrvs[info] = ptr;
    return ptr.get();
  }

  return mSrvs[info].get();

}

void RHITexture::invalidateViews() {
  mSrvs.clear();
}

const RenderTargetView* RHITexture::rtv(uint mipLevel, uint arraySlice) const {
  // EXPECTS(arraySlice < mArraySize);
  ResourceViewInfo info(mipLevel, 1, arraySlice, 1, DescriptorPool::Type::TextureSrv);

  auto kv = mRtvs.find(info);

  if (kv == mRtvs.end() && is_set(mBindingFlags, BindingFlag::RenderTarget)) {
    RenderTargetView::sptr_t ptr = RenderTargetView::create(
      shared_from_this(), info.mostDetailedMip,
      info.firstArraySlice, info.arraySize);

    ENSURES(ptr->info() == info);

    mRtvs[info] = ptr;

    return ptr.get();
  }
  return mRtvs[info].get();
}

const DepthStencilView* RHITexture::dsv(uint mipLevel) const {
  ResourceViewInfo info(mipLevel, 1, 0, 1, DescriptorPool::Type::Dsv);

  auto kv = mDsvs.find(info);

  if (kv == mDsvs.end() && is_set(mBindingFlags, BindingFlag::DepthStencil)) {
    DepthStencilView::sptr_t ptr = DepthStencilView::create(
      shared_from_this(), info.mostDetailedMip,
      info.firstArraySlice, info.arraySize);

    ENSURES(ptr->info() == info);

    mDsvs[info] = ptr;

    return ptr.get();
  }
  return mDsvs[info].get();
}

const UnorderedAccessView* RHITexture::uav(uint mipLevel) const {
  ResourceViewInfo info(mipLevel, 1, 0, depth(mipLevel), DescriptorPool::Type::TextureUav);

  auto kv = mUavs.find(info);

  if(kv == mUavs.end() && is_set(mBindingFlags, BindingFlag::UnorderedAccess)) {
    UnorderedAccessView::sptr_t ptr = UnorderedAccessView::create(
      shared_from_this(), info.mostDetailedMip,
      info.firstArraySlice, info.arraySize
    );

    ENSURES(ptr->info() == info);

    mUavs[info] = ptr;

    return ptr.get();
  }

  return mUavs[info].get();
}

void RHITexture::generateMipmap(RHIContext& ctx) {
  EXPECTS(mType == Type::Texture2D);

  for(uint i = 0; i < mMipLevels - 1; i++) {
    const ShaderResourceView* fromSrv = srv(i, 1);
    const RenderTargetView* toRtv = rtv(i+1);
    ctx.blit(*fromSrv, *toRtv);
  }
}
#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "RHITexture.hpp"


ShaderResourceView* RHITexture::srv(uint mipLevel) const {

  if (!mSrv[mipLevel] && is_set(mBindingFlags, BindingFlag::ShaderResource)) {
    mSrv[mipLevel] = ShaderResourceView::create(shared_from_this(), mipLevel);
  }

  return mSrv[mipLevel].get();
  
}

void RHITexture::invalidateViews() {
  for(auto && srv: mSrv) {
    srv = nullptr;
  }

  for(auto && rtv: mSrv) {
    rtv = nullptr;
  }
}

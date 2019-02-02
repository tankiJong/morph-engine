#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "RHITexture.hpp"


ShaderResourceView* RHITexture::srv() const {

  if (!mSrv && is_set(mBindingFlags, BindingFlag::ShaderResource)) {
    mSrv = ShaderResourceView::create(shared_from_this());
  }

  return mSrv.get();
  
}
#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "RHITexture.hpp"


const ShaderResourceView& RHITexture::srv() const {

  if (!mSrv) {
    mSrv = ShaderResourceView::create(shared_from_this());
  }

  return *mSrv;
  
}
#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "RHITexture.hpp"


ShaderResourceView& RHITexture::srv() {

  if (!mSrv) {
    mSrv = ShaderResourceView::create(shared_from_this());
  }

  return *mSrv;
  
}
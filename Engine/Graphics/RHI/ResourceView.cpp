#include "ResourceView.hpp"

ShaderResourceView::sptr_t ShaderResourceView::sNullView;
RenderTargetView::sptr_t   RenderTargetView::sNullView;
DepthStencilView::sptr_t   DepthStencilView::sNullView;
ConstantBufferView::sptr_t ConstantBufferView::sNullView;

ConstantBufferView::sptr_t ConstantBufferView::nullView() {
  if(!sNullView) {
    create(W<RHIBuffer>());
  }

  return sNullView;
}

RenderTargetView::sptr_t RenderTargetView::nullView() {
  if (!sNullView) {
    create(W<RHITexture>());
  }

  return sNullView;
}

DepthStencilView::sptr_t DepthStencilView::nullView() {
  if (!sNullView) {
    create(W<RHITexture>());
  }

  return sNullView;
}

ShaderResourceView::sptr_t ShaderResourceView::nullView() {
  if (!sNullView) {
    create(W<RHITexture>());
  }

  return sNullView;
}

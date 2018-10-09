#include "ResourceView.hpp"

ShaderResourceView::sptr_t ShaderResourceView::sNullView;
RenderTargetView::sptr_t   RenderTargetView::sNullView;
DepthStencilView::sptr_t   DepthStencilView::sNullView;
ConstantBufferView::sptr_t ConstantBufferView::sNullView;
UnorderedAccessView::sptr_t UnorderedAccessView::sNullView;

ConstantBufferView::sptr_t ConstantBufferView::nullView() {
  if(!sNullView) {
    sNullView = create(W<RHIBuffer>());
  }

  return sNullView;
}

RenderTargetView::sptr_t RenderTargetView::nullView() {
  if (!sNullView) {
    sNullView = create(W<RHITexture>());
  }

  return sNullView;
}

DepthStencilView::sptr_t DepthStencilView::nullView() {
  if (!sNullView) {
    sNullView = create(W<RHITexture>());
  }

  return sNullView;
}

ShaderResourceView::sptr_t ShaderResourceView::nullView() {
  if (!sNullView) {
    sNullView = create(W<RHITexture>());
  }

  return sNullView;
}

UnorderedAccessView::sptr_t UnorderedAccessView::nullView() {
  if(!sNullView) {
    sNullView = create(W<Texture2>());
  }

  return sNullView;
}

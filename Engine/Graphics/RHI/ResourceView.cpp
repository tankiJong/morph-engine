#include "ResourceView.hpp"

ShaderResourceView::sptr_t ShaderResourceView::sNullView;
ConstantBufferView::sptr_t ConstantBufferView::sNullView;

ConstantBufferView::sptr_t ConstantBufferView::nullView() {
  if(!sNullView) {
    create(W<RHIBuffer>());
  }

  return sNullView;
}

ShaderResourceView::sptr_t ShaderResourceView::nullView() {
  if (!sNullView) {
    create(W<RHIResource>());
  }

  return sNullView;
}

#include "FrameBuffer.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

FrameBuffer::FrameBuffer(const Desc& desc): mDesc(desc) {
  mColorTarget.fill(nullptr);
}

const FrameBuffer::Desc& FrameBuffer::desc() const {
  return mDesc;
}

FrameBuffer::FrameBuffer() {
  mColorTarget.fill(nullptr);
}

void FrameBuffer::setColorTarget(const RenderTargetView* rtv, uint index) {
  mColorTarget[index] = rtv;
}

void FrameBuffer::setColorTarget(const Texture2::sptr_t& tex, uint index) {
  
  Expects(index < NUM_MAX_COLOR_TARGET);
  EXPECTS(tex->format() == mDesc.colorTargetFormat(index));

  setColorTarget(&tex->rtv(), index);
}

void FrameBuffer::setDepthStencilTarget(Texture2::sptr_t tex) {
  EXPECTS(tex->format() == mDesc.depthTargetFormat());
  mEnableDepth = true;
  mDepthTarget = tex->dsv();
}

void FrameBuffer::setDepthStencilTarget(const DepthStencilView* dsv) {
  mDepthTarget = dsv;

};


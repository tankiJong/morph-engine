#include "FrameBuffer.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

FrameBuffer::FrameBuffer(const Desc& desc): mDesc(desc) {
  mColorTarget.fill(nullptr);
}

const FrameBuffer::Desc& FrameBuffer::desc() const {
  return mDesc;
}

FrameBuffer::Desc& FrameBuffer::desc() {
  return mDesc;
}

FrameBuffer::FrameBuffer() {
  mColorTarget.fill(nullptr);
}

void FrameBuffer::setColorTarget(const RenderTargetView* rtv, uint index) {
  mColorTarget[index] = rtv;
}

void FrameBuffer::setColorTarget(const Texture2::sptr_t& tex, uint index) {
  
  EXPECTS(index < NUM_MAX_COLOR_TARGET);

  auto expectFormat = mDesc.colorTargetFormat(index);
  EXPECTS(tex->format() ==  expectFormat || expectFormat == TEXTURE_FORMAT_UNKNOWN);

  setColorTarget(&tex->rtv(), index);
}

void FrameBuffer::defineColorTarget(const Texture2::sptr_t tex, uint index, bool allowUav) {
  mDesc.defineColorTarget(index, tex->format(), allowUav);
  setColorTarget(tex, index);
}

void FrameBuffer::defineDepthStencilTarget(const Texture2::sptr_t tex, bool allowUav) {
  mDesc.defineDepthTarget(tex->format(), allowUav);
  setDepthStencilTarget(tex);
}

void FrameBuffer::setDepthStencilTarget(Texture2::sptr_t tex) {
  EXPECTS(tex->format() == mDesc.depthTargetFormat());
  mEnableDepth = true;
  mDepthTarget = tex->dsv();
}

void FrameBuffer::setDepthStencilTarget(const DepthStencilView* dsv) {
  mDepthTarget = dsv;

}

void FrameBuffer::setDesc(const Desc& desc) {
  mDesc = desc;
};


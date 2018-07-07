#include "FrameBuffer.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

void FrameBuffer::setColorTarget(Texture2::sptr_t tex, uint index) {
  
  Expects(index < NUM_MAX_COLOR_TARGET);
  EXPECTS(tex->format() == mDesc.colorTargetFormat(index));
  mColorTarget[index] = tex;


}

void FrameBuffer::setDepthStencilTarget(Texture2::sptr_t tex) {
  EXPECTS(tex->format() == mDesc.depthTargetFormat());
  mEnableDepth = true;
  mDepthTarget = tex;
};


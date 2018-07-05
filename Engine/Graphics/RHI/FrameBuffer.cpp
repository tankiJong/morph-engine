#include "FrameBuffer.hpp"

void FrameBuffer::setColorTarget(RHITexture::sptr_t tex, uint index,
  uint mipLevel, uint firstArraySlice, uint arraySize) {
  
  Expects(index < NUM_MAX_COLOR_TARGET);

  mColorTarget[index] = tex;


}

void FrameBuffer::setDepthStencilTarget(RHITexture::sptr_t tex, 
  uint mipLevel, uint firstArraySlice, uint arraySize) {

  mEnableDepth = true;
};


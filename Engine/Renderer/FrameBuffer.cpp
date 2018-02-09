#include "Engine/Renderer/glFunctions.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "FrameBuffer.hpp"
bool FrameBuffer::finalize() {
  glBindBuffer(GL_FRAMEBUFFER, mHandle);

  uint colorTargetHandles[1];
  colorTargetHandles[0] = mColorTarget->getHandle();

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 0, 
                       mColorTarget->getHandle(), 0);

  glDrawBuffers(1, colorTargetHandles);

  if(mDepthTarget != nullptr) {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                         mDepthTarget->getHandle(), 0);
  } else {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                         NULL, 0);
  }
#if defined(_DEBUG)
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE) {
    ERROR_AND_DIE("invalid frame buffer");
    return false;
  }
#endif

  return true;
}

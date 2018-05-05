#include "Engine/Renderer/glFunctions.hpp"
#include "FrameBuffer.hpp"
#include "Engine/Renderer/RenderTarget.hpp"

FrameBuffer::FrameBuffer() {
  glGenFramebuffers(1, &mHandle);
}

FrameBuffer::~FrameBuffer() {
  if (mHandle != NULL) {
    glDeleteFramebuffers(1, &mHandle);
    mHandle = NULL;
  }
}

uint FrameBuffer::width() const {
  return mColorTarget->mDimensions.x;
}

uint FrameBuffer::height() const {
  return mColorTarget->mDimensions.y;
}

void FrameBuffer::setColorTarget(Texture* colorTarget) {
  mColorTarget = colorTarget;
}

void FrameBuffer::setDepthStencilTarget(Texture* depthTarget) {
  mDepthTarget = depthTarget;
}

bool FrameBuffer::finalize() {

  glBindFramebuffer(GL_FRAMEBUFFER, mHandle);

  // keep track of which outputs go to which attachments; 
  GLenum targets[1];

  // Bind a color target to an attachment point
  // and keep track of which locations to to which attachments. 
  glFramebufferTexture(GL_FRAMEBUFFER,
                       GL_COLOR_ATTACHMENT0 + 0,
                       mColorTarget->getHandle(),0);
  // 0 to to attachment 0
  targets[0] = GL_COLOR_ATTACHMENT0 + 0;

  // Update target bindings
  glDrawBuffers(1, targets);

  // Bind depth if available;
  if (mDepthTarget == nullptr) {
    glFramebufferTexture(GL_FRAMEBUFFER,
                         GL_DEPTH_STENCIL_ATTACHMENT,
                         NULL,0);
  } else {
    glFramebufferTexture(GL_FRAMEBUFFER,
                         GL_DEPTH_STENCIL_ATTACHMENT,
                         mDepthTarget->getHandle(),0);
  }
  
  // Error Check - recommend only doing in debug
#if defined(_DEBUG)
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    ERROR_AND_DIE("Failed to create framebuffer");
//    return false;
  }
#endif

  return true;
}

#include "Engine/Renderer/glFunctions.hpp"
#include "FrameBuffer.hpp"
#include "Engine/Renderer/RenderTarget.hpp"

FrameBuffer::FrameBuffer() {
  glGenFramebuffers(1, &mHandle);
  mColorTarget.fill(nullptr);
}

FrameBuffer::~FrameBuffer() {
  if (mHandle != NULL) {
    glDeleteFramebuffers(1, &mHandle);
    mHandle = NULL;
  }
}

uint FrameBuffer::width() const {
  if(mDepthTarget != nullptr) {
    return mDepthTarget->mDimensions.x;
  }
  return mColorTarget[0]->mDimensions.x;
}

uint FrameBuffer::height() const {
  if(mDepthTarget != nullptr) {
    return mDepthTarget->mDimensions.y;
  }
  return mColorTarget[0]->mDimensions.y;
}

void FrameBuffer::setColorTarget(Texture* colorTarget, uint slot) {
  mColorTarget[slot] = colorTarget;
}

void FrameBuffer::setDepthStencilTarget(Texture* depthTarget) {
  mDepthTarget = depthTarget;
}

bool FrameBuffer::finalize() {

  glBindFramebuffer(GL_FRAMEBUFFER, mHandle);


  // Bind a color target to an attachment point
  // and keep track of which locations to to which attachments. 

  GLenum targets[NUM_MAX_TARGET];

  for(uint i = 0; i<NUM_MAX_TARGET; ++i) {
    if (mColorTarget[i] != nullptr) {
      targets[i] = GL_COLOR_ATTACHMENT0 + i;
      // keep track of which outputs go to which attachments; 

      glFramebufferTexture(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0 + i,
                           mColorTarget[i]->getHandle(), 0);
      // 0 to to attachment 0

      // Update target bindings
    } else {
      targets[i] = GL_NONE;
    }
  }
  glDrawBuffers(NUM_MAX_TARGET, targets);

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

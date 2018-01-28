#include "RenderBuffer.hpp"
#include "glFunctions.hpp"

RenderBuffer::~RenderBuffer() {
    // cleanup for a buffer; 
    if (handle != NULL) {
      glDeleteBuffers(1, &handle);
      handle = NULL;
    }
  }

bool RenderBuffer::copyToGpu(size_t byteCount, const void* data) {
  // handle is a GLuint member - used by OpenGL to identify this buffer
  // if we don't have one, make one when we first need it [lazy instantiation]
  if (handle == NULL) {
    glGenBuffers(1, &handle);
  }

  // Bind the buffer to a slot, and copy memory
  // GL_DYNAMIC_DRAW means the memory is likely going to change a lot (we'll get
  // during the second project)
  glBindBuffer(GL_ARRAY_BUFFER, handle);
  glBufferData(GL_ARRAY_BUFFER, byteCount, data, GL_DYNAMIC_DRAW);

  // buffer_size is a size_t member variable I keep around for 
  // convenience
  bufferSize= byteCount;
  return true;

  }

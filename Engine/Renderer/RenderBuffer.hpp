#pragma once
typedef unsigned int GLuint;
class RenderBuffer {
  friend class Renderer;
public:
  ~RenderBuffer();
  bool copyToGpu(size_t byteCount, const void* data);

  GLuint handle = 0;
  size_t bufferSize = 0;

protected:
  RenderBuffer() = default;
};

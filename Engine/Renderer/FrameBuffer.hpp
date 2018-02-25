#pragma once
#include "Engine/Core/common.hpp"

class Texture;
class FrameBuffer {
public:
  uint mHandle;

  // TODO: maybe change ownership
  Texture* mColorTarget;
  Texture* mDepthTarget;
  FrameBuffer();
  ~FrameBuffer();

  uint width() const;
  uint height() const;

  void setColorTarget(Texture* colorTarget);
  void setDepthStencilTarget(Texture* depthTarget);
  bool finalize();


};

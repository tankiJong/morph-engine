#pragma once
#include "Engine/Core/common.hpp"

class Texture;
class FrameBuffer {
public:
  uint mHandle;
  Texture* mColorTarget;
  Texture* mDepthTarget;
  FrameBuffer();
  ~FrameBuffer();

  void setColorTarget(Texture* colorTarget);
  bool finalize();


};

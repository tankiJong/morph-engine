#pragma once
#include "Engine/Core/common.hpp"

class Texture;
class FrameBuffer {
public:
  uint mHandle;
  static constexpr uint NUM_MAX_TARGET = 8;

  // TODO: maybe change ownership
  std::array<Texture*, NUM_MAX_TARGET> mColorTarget;
  Texture* mDepthTarget;
  FrameBuffer();
  ~FrameBuffer();

  uint width() const;
  uint height() const;

  void setColorTarget(Texture* colorTarget, uint slot = 0);
  void setDepthStencilTarget(Texture* depthTarget);
  bool finalize();

};

#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHITexture.hpp"

class FrameBuffer {
public:
  using sptr_t = S<FrameBuffer>;
  using scptr_t = S<const FrameBuffer>;

  static constexpr uint NUM_MAX_COLOR_TARGET = 8;
  static constexpr uint kAttachEntireMipLevel = uint(-1);

  struct target_info_t {
    eTextureFormat format = TEXTURE_FORMAT_UNKNOWN;
    bool allowUav = false;
  };

  class Desc {
  public:
    Desc& defineColorTarget(uint index, eTextureFormat format, bool allowUav = false) {
      colorTargets[index] = { format, allowUav };
    }

    eTextureFormat colorTargetFormat(uint index) const {
      return colorTargets[index].format;
    }

    eTextureFormat depthTargetFormat() const {
      return depthTarget.format;
    }
    Desc& defineDepthTarget(eTextureFormat format, bool allowUav = false) {
      depthTarget = { format, allowUav };
    }

  protected:
    target_info_t colorTargets[NUM_MAX_COLOR_TARGET];
    target_info_t depthTarget;
  };

  uint width() const;
  uint height() const;

  void setColorTarget(RHITexture::sptr_t tex, uint index,
                      uint mipLevel = 0, uint firstArraySlice = 0, uint arraySize = kAttachEntireMipLevel);
  void setDepthStencilTarget(RHITexture::sptr_t tex, uint mipLevel = 0, uint firstArraySlice = 0, uint arraySize = kAttachEntireMipLevel);

protected:
  FrameBuffer(const Desc& desc) : mDesc(desc) {};
  FrameBuffer() {};
  Desc mDesc {};
  std::array<RHITexture::sptr_t, NUM_MAX_COLOR_TARGET> mColorTarget;
  RHITexture::sptr_t mDepthTarget;
  bool mEnableDepth = false;
  ~FrameBuffer();


};

#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHITexture.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"

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
      return *this;
    }

    eTextureFormat colorTargetFormat(uint index) const {
      return colorTargets[index].format;
    }

    eTextureFormat depthTargetFormat() const {
      return depthTarget.format;
    }
    Desc& defineDepthTarget(eTextureFormat format, bool allowUav = false) {
      depthTarget = { format, allowUav };
      return *this;
    }

  protected:
    target_info_t colorTargets[NUM_MAX_COLOR_TARGET];
    target_info_t depthTarget;
  };

  FrameBuffer(const Desc& desc);

  const FrameBuffer::Desc& desc() const;
  FrameBuffer::Desc& desc();

  FrameBuffer();

  uint width() const;
  uint height() const;

  void setColorTarget(const RenderTargetView* rtv, uint index);
  void setColorTarget(const Texture2::sptr_t& tex, uint index);
  void setDepthStencilTarget(Texture2::sptr_t tex);
  void setDepthStencilTarget(const DepthStencilView* dsv);
  void setDesc(const Desc& desc);

  const RenderTargetView* colorTarget(uint index) const { return mColorTarget[index]; }

  const DepthStencilView* depthStencilTarget() const { return mDepthTarget; }
protected:
  Desc mDesc {};
  std::array<const RenderTargetView*, NUM_MAX_COLOR_TARGET> mColorTarget;
  const DepthStencilView* mDepthTarget = nullptr;
  bool mEnableDepth = false;


};

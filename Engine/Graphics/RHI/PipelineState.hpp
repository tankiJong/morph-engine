#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RootSignature.hpp"
#include "Engine/Graphics/RHI/VertexLayout.hpp"
#include "Engine/Graphics/RHI/FrameBuffer.hpp"

class PipelineState {
public:
  using sptr_t = S<PipelineState>;
  using rhi_handle_t = pipeline_state_handle_t;
  static const uint SAMPLE_MASK_ALL = UINT_MAX;

  enum class PrimitiveType {
    Undefined = -1,
    Point,
    Line,
    Triangle,
    Patch,
  };

  class Desc {
  public:
    Desc& setRootSignature(RootSignature::sptr_t signature) {
      mRootSignature = signature; return *this;
    }

    Desc& setVertexLayout(const VertexLayout* layout) {
      mLayout = layout; return *this;
    }

    Desc& setRenderState(const render_state state) {
      mRenderState = state; return *this;
    }

    Desc& setPrimTye(const PrimitiveType prim) {
      mPrimType = prim; return *this;
    }

    Desc& setFboDesc(const FrameBuffer::Desc& desc) {
      mFboDesc = desc; return *this;
    }

    bool operator==(const Desc& rhs) const;
  protected:
    friend class PipelineState;

    const VertexLayout* mLayout = nullptr;
    RootSignature::sptr_t mRootSignature;
    render_state mRenderState;
    uint mSampleMask = SAMPLE_MASK_ALL;
    FrameBuffer::Desc mFboDesc;
    PrimitiveType mPrimType = PrimitiveType::Undefined;
  };

  ~PipelineState();
  static sptr_t create(const Desc& desc);
  rhi_handle_t handle() { return mRhiHandle; }
  RootSignature::scptr_t rootSignature() const { return mDesc.mRootSignature; }
protected:
  PipelineState(const Desc& desc) : mDesc(desc) {}
  bool rhiInit();
  Desc mDesc;
  rhi_handle_t mRhiHandle;
};

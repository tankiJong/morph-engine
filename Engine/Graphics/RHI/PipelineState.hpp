#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RootSignature.hpp"
#include "Engine/Graphics/RHI/FrameBuffer.hpp"

class VertexLayout;
class Program;

class GraphicsState {
public:
  using sptr_t = S<GraphicsState>;
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
    Desc& setRootSignature(RootSignature::scptr_t signature) {
      mRootSignature = signature; return *this;
    }

    Desc& setVertexLayout(const VertexLayout* layout) {
      mLayout = layout; return *this;
    }

    Desc& setPrimTye(const PrimitiveType prim) {
      mPrimType = prim; return *this;
    }

    Desc& setFboDesc(const FrameBuffer::Desc& desc) {
      mFboDesc = desc; return *this;
    }

    Desc& setProgram(const S<Program>& prog) {
      mProgram = prog; return *this;
    }

    bool operator==(const Desc& rhs) const;
  protected:
    friend class GraphicsState;

    const VertexLayout* mLayout = nullptr;
    RootSignature::scptr_t mRootSignature;
    uint mSampleMask = SAMPLE_MASK_ALL;
    FrameBuffer::Desc mFboDesc;
    PrimitiveType mPrimType = PrimitiveType::Undefined;
    S<Program> mProgram;
  };

  ~GraphicsState();
  static sptr_t create(const Desc& desc);
  rhi_handle_t handle() const { return mRhiHandle; }
  RootSignature::scptr_t rootSignature() const { return mDesc.mRootSignature; }
protected:
  GraphicsState(const Desc& desc) : mDesc(desc) {}
  bool rhiInit();
  Desc mDesc;
  rhi_handle_t mRhiHandle;
};


class ComputeState {
public:
  using sptr_t = S<ComputeState>;
  using rhi_handle_t = pipeline_state_handle_t;

  class Desc {
    friend class ComputeState;
  public:
    Desc& setRootSignature(RootSignature::sptr_t sig) {
      mRootSignature = sig; return *this;
    };
  protected:
    RootSignature::sptr_t mRootSignature;

  };

  static sptr_t create(const Desc& desc);

  rhi_handle_t handle() const { return mRhiHandle; }
  RootSignature::scptr_t rootSignature() const { return mDesc.mRootSignature; }
protected:
  ComputeState(const Desc& desc) : mDesc(desc) {}

  bool rhiInit();
  Desc mDesc;
  rhi_handle_t mRhiHandle;
};

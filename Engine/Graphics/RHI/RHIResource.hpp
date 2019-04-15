#pragma once

#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/RHI/RHIHeap.hpp"

// code structure idea adpot from Falcor Engine: Source/API/Resource.h

class RHIResource: public std::enable_shared_from_this<RHIResource> {
  friend class RHIContext;
public:
  using handle_t = rhi_resource_handle_t;
  using sptr_t = S<RHIResource>;
  using scptr_t = S<const RHIResource>;

  /** These flags are hints the driver to what pipeline stages the resource will be bound to. 
   */
  enum class BindingFlag: uint {
    None = 0x0,             ///< The resource will not be bound the pipeline. Use this to create a staging resource
    VertexBuffer = 0x1,     ///< The resource will be bound as a vertex-buffer
    IndexBuffer = 0x2,      ///< The resource will be bound as a index-buffer
    ConstantBuffer = 0x4,   ///< The resource will be bound as a constant-buffer
    StreamOutput = 0x8,     ///< The resource will be bound to the stream-output stage as an output buffer
    ShaderResource = 0x10,  ///< The resource will be bound as a shader-resource
    UnorderedAccess = 0x20, ///< The resource will be bound as an UAV
    RenderTarget = 0x40,    ///< The resource will be bound as a render-target
    DepthStencil = 0x80,    ///< The resource will be bound as a depth-stencil buffer
    IndirectArg = 0x100,    ///< The resource will be bound as an indirect argument buffer
#ifdef MORPH_DXR
    AccelerationStructure = 0x80000000,
#endif
  };

  /** Resource types. Notice there are no array types. Array are controlled using the array size parameter on texture creation.
   */
  enum class Type: uint {
    Unknown,
    Buffer,                 ///< Buffer. Can be bound to all shader-stages
    Texture1D,              ///< 1D texture. Can be bound as render-target, shader-resource and UAV
    Texture2D,              ///< 2D texture. Can be bound as render-target, shader-resource and UAV
    Texture3D,              ///< 3D texture. Can be bound as render-target, shader-resource and UAV
    TextureCube,            ///< Texture-cube. Can be bound as render-target, shader-resource and UAV
    Texture2DMultisample,   ///< 2D multi-sampled texture. Can be bound as render-target, shader-resource and UAV
  };

  /** Resource state. Keeps track of how the resource was last used
  */
  enum class State : uint {
    Undefined,
    PreInitialized,
    Common,
    VertexBuffer,
    ConstantBuffer,
    IndexBuffer,
    RenderTarget,
    UnorderedAccess,
    DepthStencil,
    ShaderResource,
    StreamOut,
    IndirectArg,
    CopyDest,
    CopySource,
    ResolveDest,
    ResolveSource,
    Present,
    GenericRead,
    Predication,
    NonPixelShader,
#ifdef MORPH_DXR
    AccelerationStructure,
#endif
  };

  handle_t handle() const { return mRhiHandle; };

  State globalState() const {
    EXPECTS(mState.global);
    return mState.globalState;
  }
  bool isStateGlobal() const { return mState.global; }

  void setGlobalState(State newState) const;
  void markGlobalInTransition(bool inTransition) const;
  inline Type type() const { return mType; }
  inline BindingFlag flags() const { return mBindingFlags; }
  virtual const UnorderedAccessView* uav(uint mipLevel = 0) const { UNUSED(mipLevel); return nullptr; };
  virtual const ConstantBufferView* cbv() const { return nullptr; };
  virtual const ShaderResourceView* srv(uint mipLevel = 0, uint32_t mipCount = ResourceViewInfo::MAX_POSSIBLE) const { UNUSED(mipLevel); UNUSED(mipCount); return nullptr; };
  virtual const RenderTargetView* rtv(uint mipLevel = 0, uint arraySlice = 0) const { UNUSED(mipLevel); UNUSED(arraySlice);return nullptr; }
  virtual const DepthStencilView* dsv(uint mipLevel = 0) const { UNUSED(mipLevel); return nullptr; }
  virtual ~RHIResource();

protected:
  RHIResource(Type type, BindingFlag bindings): mType(type), mBindingFlags(bindings) {}
  RHIResource(rhi_resource_handle_t res);

  mutable bool mInTransition = false;

  handle_t mRhiHandle;
  RHIHeap::sptr_t mGpuMemory = nullptr;
  Type mType;
  BindingFlag mBindingFlags;
  mutable UnorderedAccessView::sptr_t mUav;
  mutable ConstantBufferView::sptr_t mCbv;

  struct {
    bool global = true;
    State globalState = State::Undefined;
    bool globalInTransition = false;
    std::vector<State> subresourceState;
    std::vector<bool> subresourceInTransition;
  } mutable mState;
};

inline void RHIResource::setGlobalState(State newState) const {
  // EXPECTS(mState.global);
  mState.globalState = newState;
}

inline void RHIResource::markGlobalInTransition(bool inTransition) const {
  EXPECTS(mState.global);
  mState.globalInTransition = inTransition;
}

void setName(const RHIResource& res, const wchar_t* name);

#define NAME_RHIRES(res) setName(*res, L#res)
enum_class_operators(RHIResource::BindingFlag);


template<typename T>
struct rhi_type {
  static constexpr RHIResource::Type value = RHIResource::Type::Unknown;
  static constexpr bool valid = false;
};

template<>
struct rhi_type<RHIResource> {
  static constexpr RHIResource::Type value = RHIResource::Type::Unknown;
  static constexpr bool valid = true;
};

template<>
struct rhi_type<Texture2> {
  static constexpr RHIResource::Type value = RHIResource::Type::Texture2D;
  static constexpr bool valid = true;
};

class Texture3;
template<>
struct rhi_type<Texture3> {
  static constexpr RHIResource::Type value = RHIResource::Type::Texture3D;
  static constexpr bool valid = true;
};

class TextureCube;
template<>
struct rhi_type<TextureCube> {
  static constexpr RHIResource::Type value = RHIResource::Type::TextureCube;
  static constexpr bool valid = true;
};

template<>
struct rhi_type<RHIBuffer> {
  static constexpr RHIResource::Type value = RHIResource::Type::Buffer;
  static constexpr bool valid = true;
};

template<typename T>
struct rhi_type_t {
  static constexpr RHIResource::Type value = rhi_type<std::decay_t<T>>::value;
  static constexpr bool valid = rhi_type<std::decay_t<T>>::valid;
};

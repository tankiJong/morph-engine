#include "Dx12Resource.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

const D3D12_HEAP_PROPERTIES DefaultHeapProps =
{
  D3D12_HEAP_TYPE_DEFAULT,
  D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
  D3D12_MEMORY_POOL_UNKNOWN,
  0,
  0
};

const D3D12_HEAP_PROPERTIES UploadHeapProps =
{
  D3D12_HEAP_TYPE_UPLOAD,
  D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
  D3D12_MEMORY_POOL_UNKNOWN,
  0,
  0,
};

const D3D12_HEAP_PROPERTIES ReadbackHeapProps =
{
  D3D12_HEAP_TYPE_READBACK,
  D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
  D3D12_MEMORY_POOL_UNKNOWN,
  0,
  0
};

D3D12_RESOURCE_FLAGS asD3D12ResourceFlags(RHIResource::BindingFlag flags) {
  D3D12_RESOURCE_FLAGS d3d = D3D12_RESOURCE_FLAG_NONE;

  bool uavRequired = is_set(flags, RHIResource::BindingFlag::UnorderedAccess);
#ifdef MORPH_DXR
  uavRequired = uavRequired || is_set(flags, RHIResource::BindingFlag::AccelerationStructure);
#endif
  if (uavRequired) {
    d3d |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }

  if (is_set(flags, RHIResource::BindingFlag::DepthStencil)) {
    if (is_set(flags, RHIResource::BindingFlag::ShaderResource) == false) {
      d3d |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }
    d3d |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  }

  if (is_set(flags, RHIResource::BindingFlag::RenderTarget)) {
    d3d |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  }

  return d3d;
}

D3D12_RESOURCE_FLAGS asDx12ResourceFlags(RHIResource::BindingFlag flags) {
  D3D12_RESOURCE_FLAGS d3d = D3D12_RESOURCE_FLAG_NONE;

  bool uavRequired = is_set(flags, RHIResource::BindingFlag::UnorderedAccess);

  if (uavRequired) {
    d3d |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }

  if (is_set(flags, RHIResource::BindingFlag::DepthStencil)) {
    if (is_set(flags, RHIResource::BindingFlag::ShaderResource) == false) {
      d3d |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }
    d3d |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  }

  if (is_set(flags, RHIResource::BindingFlag::RenderTarget)) {
    d3d |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  }

  return d3d;
}

RHIResource::BindingFlag asRHIResourceFlags(D3D12_RESOURCE_FLAGS flags) {
  RHIResource::BindingFlag rhiFlag = RHIResource::BindingFlag::None;
  if (flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
    rhiFlag |= RHIResource::BindingFlag::RenderTarget;

  if (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
    rhiFlag |= RHIResource::BindingFlag::DepthStencil;
    if((flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == false) {
      rhiFlag |= RHIResource::BindingFlag::ShaderResource;
    }
  }

  if (flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
    rhiFlag |= RHIResource::BindingFlag::UnorderedAccess;

  if (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
    rhiFlag |= RHIResource::BindingFlag::DepthStencil;

  if (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
    rhiFlag |= RHIResource::BindingFlag::DepthStencil;

  return rhiFlag;
}

D3D12_RESOURCE_STATES asDx12ResourceState(RHIResource::State state) {
  switch (state) {
    case RHIResource::State::Undefined:
    case RHIResource::State::Common:
      return D3D12_RESOURCE_STATE_COMMON;
    case RHIResource::State::ConstantBuffer:
    case RHIResource::State::VertexBuffer:
      return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case RHIResource::State::CopyDest:
      return D3D12_RESOURCE_STATE_COPY_DEST;
    case RHIResource::State::CopySource:
      return D3D12_RESOURCE_STATE_COPY_SOURCE;
    case RHIResource::State::DepthStencil:
      return D3D12_RESOURCE_STATE_DEPTH_WRITE; // If depth-writes are disabled, return D3D12_RESOURCE_STATE_DEPTH_WRITE
    case RHIResource::State::IndexBuffer:
      return D3D12_RESOURCE_STATE_INDEX_BUFFER;
    case RHIResource::State::IndirectArg:
      return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    case RHIResource::State::Predication:
      return D3D12_RESOURCE_STATE_PREDICATION;
    case RHIResource::State::Present:
      return D3D12_RESOURCE_STATE_PRESENT;
    case RHIResource::State::RenderTarget:
      return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case RHIResource::State::ResolveDest:
      return D3D12_RESOURCE_STATE_RESOLVE_DEST;
    case RHIResource::State::ResolveSource:
      return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    case RHIResource::State::ShaderResource:
      return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // Need the shader usage mask in case the SRV is used by non-PS
    case RHIResource::State::StreamOut:
      return D3D12_RESOURCE_STATE_STREAM_OUT;
    case RHIResource::State::UnorderedAccess:
      return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case RHIResource::State::GenericRead:
      return D3D12_RESOURCE_STATE_GENERIC_READ;
    case RHIResource::State::NonPixelShader:
      return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
#ifdef MORPH_DXR
    case RHIResource::State::AccelerationStructure:
      return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
#endif
    default:
      ERROR_AND_DIE("unexpected state type");
  }
}


RHIResource::RHIResource(rhi_resource_handle_t res) {
  mRhiHandle = res;
  D3D12_RESOURCE_DESC desc = res->GetDesc();

  switch (desc.Dimension) {
    case D3D12_RESOURCE_DIMENSION_UNKNOWN:
      mType = Type::Unknown;
      break;
    case D3D12_RESOURCE_DIMENSION_BUFFER:
      mType = Type::Buffer;
      break;
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
      mType = Type::Texture1D;
      break;
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
      mType = Type::Texture2D;
      break;
    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
      mType = Type::Texture3D;
      break;
    default:
      ERROR_AND_DIE("should not reach here");
  };

  mBindingFlags = asRHIResourceFlags(desc.Flags);
}

void setName(const RHIResource& res, const wchar_t* name) {
  res.handle()->SetName(name);
}

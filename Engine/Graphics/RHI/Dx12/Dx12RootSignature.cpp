#include "Engine/Graphics/RHI/RootSignature.hpp"
#include <vector>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Graphics/RHI/Dx12/dx12util.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

uint mSizeInByte;

D3D12_SHADER_VISIBILITY asDx12ShaderVisibility(ShaderVisibility visibility) {
  // D3D12 doesn't support a combination of flags, it's either ALL or a single stage
  if (isPow2(visibility) == false) {
    return D3D12_SHADER_VISIBILITY_ALL;
  } else if ((visibility & ShaderVisibility::Vertex) != ShaderVisibility::None) {
    return D3D12_SHADER_VISIBILITY_VERTEX;
  } else if ((visibility & ShaderVisibility::Pixel) != ShaderVisibility::None) {
    return D3D12_SHADER_VISIBILITY_PIXEL;
  // } else if ((visibility & ShaderVisibility::Geometry) != ShaderVisibility::None) {
  //   return D3D12_SHADER_VISIBILITY_GEOMETRY;
  // } else if ((visibility & ShaderVisibility::Domain) != ShaderVisibility::None) {
  //   return D3D12_SHADER_VISIBILITY_DOMAIN;
  // } else if ((visibility & ShaderVisibility::Hull) != ShaderVisibility::None) {
  //   return D3D12_SHADER_VISIBILITY_HULL;
  }
  // If it was compute, it can't be anything else and so the first `if` would have handled it
  ERROR_AND_DIE("unexpected logic");
  return (D3D12_SHADER_VISIBILITY)-1;
}

D3D12_DESCRIPTOR_RANGE_TYPE asDx12RangeType(const RootSignature::desc_type_t type) {
  switch(type) {
    case RootSignature::desc_type_t::TextureSrv:
    case RootSignature::desc_type_t::TypedBufferSrv:
    case RootSignature::desc_type_t::StructuredBufferSrv:
      return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    case RootSignature::desc_type_t::TextureUav:
    case RootSignature::desc_type_t::TypedBufferUav:
    case RootSignature::desc_type_t::StructuredBufferUav:
      return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    case RootSignature::desc_type_t::Cbv:
      return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    case RootSignature::desc_type_t::Sampler:
      return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    default:
      ERROR_AND_DIE("unexpected logic");
      return (D3D12_DESCRIPTOR_RANGE_TYPE)-1;
  }
}

auto asDesctiptionTable(const RootSignature::desc_set_layout_t& set, D3D12_ROOT_PARAMETER& desc, std::vector<D3D12_DESCRIPTOR_RANGE>& range) {

  desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  desc.ShaderVisibility = asDx12ShaderVisibility(set.visibility());
  range.resize(set.rangeCount());
  desc.DescriptorTable.NumDescriptorRanges = (uint)set.rangeCount();
  desc.DescriptorTable.pDescriptorRanges = range.data();

  for(size_t i = 0; i < set.rangeCount(); i++) {
    const auto& r = set.range(i);
    range[i].BaseShaderRegister = r.baseRegisterIndex;
    range[i].NumDescriptors = r.descCount;
    range[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    range[i].RangeType = asDx12RangeType(r.type);
    range[i].RegisterSpace = r.registerSpace;
  }
}

bool RootSignature::rhiInit() {
  mSizeInByte = 0;
  std::vector<D3D12_ROOT_PARAMETER> rootParams(mDesc.mSets.size());
  mElementByteOffset.resize(mDesc.mSets.size());

  std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> d3dRanges(mDesc.mSets.size());

  for(size_t i = 0; i<mDesc.mSets.size(); i++) {
    const desc_set_layout_t& set = mDesc.mSets[i];
    asDesctiptionTable(set, rootParams[i], d3dRanges[i]);
    mElementByteOffset[i] = mSizeInBytes;
    constexpr auto s = sizeof(D3D12_ROOT_PARAMETER);
    // QA: why this is 8..
    mSizeInBytes += 8;
  }

  D3D12_ROOT_SIGNATURE_DESC desc;

  desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

  // provide a default static sampler to pixel shader
  D3D12_STATIC_SAMPLER_DESC sampler = {};
  sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
  sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.MipLODBias = 0;
  sampler.MaxAnisotropy = 0;
  sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
  sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
  sampler.MinLOD = 0.0f;
  sampler.MaxLOD = D3D12_FLOAT32_MAX;
  sampler.ShaderRegister = 0;
  sampler.RegisterSpace = 0;
  sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

  desc.pParameters = rootParams.data();
  desc.NumParameters = (uint)rootParams.size();
  desc.pStaticSamplers = &sampler;
  desc.NumStaticSamplers = 1;

  ID3DBlobPtr sigBlob;
  ID3DBlobPtr errBlob;

  d3d_call(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &sigBlob, &errBlob));

  if(mSizeInBytes > sizeof(uint) + D3D12_MAX_ROOT_COST) {
    ERROR_AND_DIE("Root-signature cost is too high. D3D12 root-signatures are limited to 64 DWORDs, trying to create a signature with" + std::to_string(mSizeInBytes / sizeof(uint)) + "DWORDS");

    return false;
  }

  initHandle(sigBlob);

}

void RootSignature::initHandle(ID3DBlobPtr sigBlob) {
  RHIDevice::rhi_handle_t device = RHIDevice::get()->nativeDevice();

  d3d_call(device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&mRhiHandle)));
}

void RootSignature::initHandle(const Blob& sigBlob) {

  RHIDevice::rhi_handle_t device = RHIDevice::get()->nativeDevice();

  d3d_call(device->CreateRootSignature(0, sigBlob, sigBlob.size(), IID_PPV_ARGS(&mRhiHandle)));
}


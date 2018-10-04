#pragma once
#include "dx12util.hpp"
#include "Engine/Graphics/RHI/RootSignature.hpp"

D3D12_SHADER_VISIBILITY asDx12ShaderVisibility(ShaderVisibility visibility);

D3D12_DESCRIPTOR_RANGE_TYPE asDx12RangeType(const RootSignature::desc_type_t type);

ShaderVisibility asShaderVisibility(D3D12_SHADER_VISIBILITY visibility);

RootSignature::desc_type_t asRangeType(const D3D12_DESCRIPTOR_RANGE_TYPE type);
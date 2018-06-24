#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"

extern const D3D12_HEAP_PROPERTIES DefaultHeapProps;
extern const D3D12_HEAP_PROPERTIES UploadHeapProps;
extern const D3D12_HEAP_PROPERTIES ReadbackHeapProps;

D3D12_RESOURCE_FLAGS asDx12ResourceFlags(RHIResource::BindingFlag flags);

D3D12_RESOURCE_STATES asDx12ResourceState(RHIResource::State state);
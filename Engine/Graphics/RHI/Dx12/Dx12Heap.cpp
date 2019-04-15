#include "Engine/Graphics/RHI/RHIHeap.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"



RHIHeap::sptr_t RHIHeap::create(size_t minSize, heap_properties_t props, eUsage usage, size_t alignment) {
  RHIHeap::sptr_t ptr(new RHIHeap());

  if(alignment == 0) {
    alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
  }
  ptr->mSize = align_to(alignment, minSize);
  D3D12_HEAP_DESC desc;
  desc.SizeInBytes = ptr->mSize;
  desc.Properties = props;
  desc.Alignment = alignment;

  switch(usage) { 
    case USAGE_BUFFER: 
      desc.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
    break;
    case USAGE_TEXTURE_NON_RT_DS:
      desc.Flags = D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
    break;
    case USAGE_TEXTURE_RT_DS: 
      desc.Flags = D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
    break;
  }
  d3d_call(RHIDevice::get()->nativeDevice()->CreateHeap(&desc, IID_PPV_ARGS(&ptr->mHandle)));

  return ptr;
}

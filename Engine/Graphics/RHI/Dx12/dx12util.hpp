#pragma once
#include "d3d12.h"
#include <string>
#include "Engine/Renderer/type.h"
#include <comdef.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <dxgiformat.h>

#define EXPECT_HR_SUCCESSED(hr) EXPECTS(SUCCEEDED(hr))

void d3dTraceHR(const std::string& msg, HRESULT hr);

#ifdef _DEBUG
#define d3d_call(a) {HRESULT hr_ = (a); if(FAILED(hr_)) { d3dTraceHR( #a, hr_); }}
#else
#define d3d_call(a) a
#endif


#define MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))

MAKE_SMART_COM_PTR(IDXGIFactory4);
MAKE_SMART_COM_PTR(IDXGISwapChain3);
MAKE_SMART_COM_PTR(ID3DBlob);


MAKE_SMART_COM_PTR(ID3D12Device);
MAKE_SMART_COM_PTR(ID3D12Debug);
MAKE_SMART_COM_PTR(ID3D12CommandQueue);
MAKE_SMART_COM_PTR(ID3D12CommandAllocator);
MAKE_SMART_COM_PTR(ID3D12GraphicsCommandList);
MAKE_SMART_COM_PTR(ID3D12DescriptorHeap);
MAKE_SMART_COM_PTR(ID3D12Resource);
MAKE_SMART_COM_PTR(ID3D12Fence);
MAKE_SMART_COM_PTR(ID3D12PipelineState);
MAKE_SMART_COM_PTR(ID3D12ShaderReflection);
MAKE_SMART_COM_PTR(ID3D12RootSignature);
MAKE_SMART_COM_PTR(ID3D12QueryHeap);
MAKE_SMART_COM_PTR(ID3D12CommandSignature);
MAKE_SMART_COM_PTR(IUnknown);

std::wstring make_wstring(const std::string& str);

DXGI_FORMAT toDXGIFormat(eDataDeclType declType, uint count, bool normalized);


/*****************RHI typedef********************************/
using window_handle_t = HWND;
using rhi_obj_handle_t = IUnknownPtr;
using device_handle_t = ID3D12DevicePtr;
using rhi_resource_handle_t = ID3D12ResourcePtr;
using root_signature_handle_t = ID3D12RootSignaturePtr;
using pipeline_state_handle_t = ID3D12PipelineStatePtr;
using fence_handle_t = ID3D12FencePtr;
using command_queue_handle_t = ID3D12CommandQueuePtr;
using swapchain_handle_t = IDXGISwapChain3Ptr;
using command_list_handle_t = ID3D12GraphicsCommandListPtr;
using command_alloctor_handle_t = ID3D12CommandAllocatorPtr;
using heap_cpu_handle_t = D3D12_CPU_DESCRIPTOR_HANDLE;
using heap_gpu_handle_t = D3D12_GPU_DESCRIPTOR_HANDLE;
using descriptor_heap_handle_t = ID3D12DescriptorHeapPtr;
using descriptor_set_rhi_handle_t = void*;

using eCommandQueueType = D3D12_COMMAND_LIST_TYPE;
/*************************************************/

template<typename BlobType>
inline std::string convertBlobToString(BlobType* pBlob) {
  std::vector<char> infoLog(pBlob->GetBufferSize() + 1);
  memcpy(infoLog.data(), pBlob->GetBufferPointer(), pBlob->GetBufferSize());
  infoLog[pBlob->GetBufferSize()] = 0;
  return std::string(infoLog.data());
}


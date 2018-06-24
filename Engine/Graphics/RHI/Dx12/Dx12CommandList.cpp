#include "Dx12CommandList.hpp"
#include "d3d12.h"
#include <d3dcompiler.h>
#include "dx12util.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12Output.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12Device.hpp"
#include "Engine/File/Path.hpp"
#include "Engine/File/Utils.hpp"
#include "Engine/Graphics/RHI/Dx12/RootSignature.hpp"
#include "Engine/Graphics/RHI/Dx12/PipelineState.hpp"
#include "Engine/Renderer/Geometry/Mesh.hpp"

void Dx12CommandList::beginFrame() {
  mCommandList->Reset(mCommandAllocator.get(), (ID3D12PipelineState*)mPipelineState->native());
  mCommandList->SetGraphicsRootSignature((ID3D12RootSignature*)mRootSignature->native());

  // possibly should already been setup somewhere else
  //ID3D12DescriptorHeap* ppHeaps[] = { mSrvHeap.get() };
  //mCommandList->SetDescriptorHeaps(countof(ppHeaps), ppHeaps);
  //mCommandList->SetGraphicsRootDescriptorTable(
  //           0, mSrvHeap->GetGPUDescriptorHandleForHeapStart());

  mCommandList->RSSetViewports(1, &mDevice->getOutput()->mViewport);
  mCommandList->RSSetScissorRects(1, &mDevice->getOutput()->mScissorRect);

  D3D12_RESOURCE_BARRIER result;
  ZeroMemory(&result, sizeof(result));
  D3D12_RESOURCE_BARRIER &barrier = result;
  result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  result.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // default to this
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES; // default to this
  barrier.Transition.pResource = mDevice->getOutput()->getCurrentBackBuffer();
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

  // Indicate that the back buffer will be used as a render target.
  mCommandList->ResourceBarrier(1, &result);

  auto rtvhandle = mDevice->getRtvHandle();
  mCommandList->OMSetRenderTargets(1, &rtvhandle, FALSE, nullptr);
  clearScreen();
  mCommandList->Close();
  mDevice->dispatchCommandList(*this);

}

void Dx12CommandList::drawVertexArray(span<vertex_pcu_t>) {

  uint numVerts = 3;

  vec3 pos[3] = {
    { 0.0f, 0.25f, 0.0f },
    { 0.25f, -0.25f, 0.0f },
    { 0.f, -0.25f, 0.0f }
  };
  Rgba color[3] = {
    Rgba::red,
    Rgba::green,
    Rgba::blue,
  };

  vec2 uvs[3] = {
    vec2::right,
    vec2::zero,
    vec2::top
  };

  byte_t* verts[3] = {
    (byte_t*)pos,
    (byte_t*)color,
    (byte_t*)uvs,
  };

  uint stride[3] = { sizeof(vec3), sizeof(Rgba), sizeof(vec2) };

  DXGI_FORMAT formats[3] = { DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32_FLOAT };

  mPipelineState->setInputLayout(*VertexLayout::For<vertex_pcu_t>());
  mPipelineState->finalize();
  mCommandList->Reset(mCommandAllocator.get(), nullptr);
  mCommandList->SetGraphicsRootSignature((ID3D12RootSignature*)mRootSignature->native());
  mCommandList->SetPipelineState((ID3D12PipelineState*)mPipelineState->native());
  // mCommandList->Reset(mCommandAllocator.get(), (ID3D12PipelineState*)mPipelineState->native());

  // possibly should already been setup somewhere else
  //ID3D12DescriptorHeap* ppHeaps[] = { mSrvHeap.get() };
  //mCommandList->SetDescriptorHeaps(countof(ppHeaps), ppHeaps);
  //mCommandList->SetGraphicsRootDescriptorTable(
  //           0, mSrvHeap->GetGPUDescriptorHandleForHeapStart());

  mCommandList->RSSetViewports(1, &mDevice->getOutput()->mViewport);
  mCommandList->RSSetScissorRects(1, &mDevice->getOutput()->mScissorRect);

  D3D12_RESOURCE_BARRIER result;
  ZeroMemory(&result, sizeof(result));
  D3D12_RESOURCE_BARRIER &barrier = result;
  result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  result.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // default to this
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES; // default to this
  barrier.Transition.pResource = mDevice->getOutput()->getCurrentBackBuffer();
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

  // Indicate that the back buffer will be used as a render target.
  mCommandList->ResourceBarrier(1, &result);

  auto rtvhandle = mDevice->getRtvHandle();
  mCommandList->OMSetRenderTargets(1, &rtvhandle, FALSE, nullptr);
  ID3D12Device* nativeDevice = (ID3D12Device*)mDevice->nativeDevice();

  // set up vertex buffer stream
  ID3D12Resource* vertexBuffer[3];
  D3D12_HEAP_PROPERTIES heapProp;
  heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
  heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heapProp.CreationNodeMask = 1;
  heapProp.VisibleNodeMask = 1;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView[3];
  for(uint i = 0; i<3; ++i) {
    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = numVerts * stride[i];
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
  breakOnFail(nativeDevice->CreateCommittedResource(
    &heapProp,
    D3D12_HEAP_FLAG_NONE,
    &resourceDesc,
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&vertexBuffer[i])));

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;
    D3D12_RANGE readRange;
    readRange.Begin = 0;
    readRange.End = 0;  // We do not intend to read from this resource on the CPU.
    breakOnFail(vertexBuffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, (void*)verts[i], numVerts * stride[i]);
    vertexBuffer[i]->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    vertexBufferView[i].BufferLocation = vertexBuffer[i]->GetGPUVirtualAddress();
    vertexBufferView[i].StrideInBytes = stride[i];
    vertexBufferView[i].SizeInBytes = stride[i] * numVerts;
    mCommandList->IASetVertexBuffers(i, 1, &vertexBufferView[i]);
  }


  mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  mCommandList->DrawInstanced(3, 1, 0, 0);

  mCommandList->Close();

  mDevice->dispatchCommandList(*this);

}

void Dx12CommandList::clearScreen() {
  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  mCommandList->ClearRenderTargetView(mDevice->getRtvHandle(), clearColor, 0, nullptr);
}

void Dx12CommandList::afterFrame() {
  mDevice->mOutput->present();

  waitForPreviousFrame();
}

Dx12CommandList::Dx12CommandList(S<Dx12Device> device)
  : mDevice(device) {
  ID3D12Device* nativeDevice = (ID3D12Device*)mDevice->nativeDevice();
  /****************************************
  *
  * create empty root signature
  * https://msdn.microsoft.com/en-us/library/windows/desktop/mt709155(v=vs.85).aspx
  *
  ****************************************/

  mRootSignature.reset(new RootSignature(mDevice));

  mRootSignature->finalize(L"default", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  //// this part is in the dx example, so I will just put it here for now before I have good enough understanding to remove it.
  //D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
  //// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
  //featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
  //ID3D12Device* nativeDevice = (ID3D12Device*)mDevice->nativeDevice();
  //if (HR_FAILED(nativeDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
  //  featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
  //}

  //D3D12_DESCRIPTOR_RANGE1 ranges[1];
  //ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  //ranges[0].NumDescriptors = 1;
  //ranges[0].BaseShaderRegister = 0;
  //ranges[0].RegisterSpace = 0;
  //ranges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
  //ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

  //D3D12_ROOT_PARAMETER1 rootPararms[1];
  //rootPararms[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  //rootPararms[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  //rootPararms[0].DescriptorTable.NumDescriptorRanges = 1;
  //rootPararms[0].DescriptorTable.pDescriptorRanges = ranges;

  //D3D12_STATIC_SAMPLER_DESC sampler = {};
  //sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
  //sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  //sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  //sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  //sampler.MipLODBias = 0;
  //sampler.MaxAnisotropy = 0;
  //sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
  //sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
  //sampler.MinLOD = 0.0f;
  //sampler.MaxLOD = D3D12_FLOAT32_MAX;
  //sampler.ShaderRegister = 0;
  //sampler.RegisterSpace = 0;
  //sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

  //D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
  //rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
  //rootSignatureDesc.Desc_1_1.NumParameters = 1;
  //rootSignatureDesc.Desc_1_1.pParameters = rootPararms;
  //rootSignatureDesc.Desc_1_1.NumStaticSamplers = 1;
  //rootSignatureDesc.Desc_1_1.pStaticSamplers = &sampler;
  //rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

  //ID3DBlob* signature = nullptr;
  //ID3DBlob* error = nullptr;

  //breakOnFail(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));

  // ID3D12RootSignature* rs = mRootSignature.get();

  //breakOnFail(nativeDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rs)));

  //mRootSignature.reset(rs);

  /****************************************
  *
  * create pipeline state
  *
  ****************************************/
  mPipelineState.reset(new PipelineState(mDevice));
  mPipelineState->setRootSignature(mRootSignature);

  /****************************************
  *
  * create command list
  * https://msdn.microsoft.com/en-us/library/windows/desktop/dn899205(v=vs.85).aspx#creating_command_lists
  *
  ****************************************/
  ID3D12CommandAllocator* ca = mCommandAllocator.get();
  breakOnFail(nativeDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&ca)));
  mCommandAllocator.reset(ca);

  ID3D12GraphicsCommandList* cl = mCommandList.get();
  breakOnFail(nativeDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.get(), (ID3D12PipelineState*)mPipelineState->native(), IID_PPV_ARGS(&cl)));
  mCommandList.reset(cl);
  mCommandList->Close();


  mFenceValue = 1;
  
  waitForPreviousFrame();
}

void Dx12CommandList::waitForPreviousFrame() {
   // mDevice->waitForFence(mFenceValue);

   //  WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
   //  This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
   //  sample illustrates how to use fences for efficient resource usage and to
   //  maximize GPU utilization.
   //
   //  Signal and increment the fence value.

    const UINT64 fence = mFenceValue;
    breakOnFail(mDevice->mCommandQueue->Signal(mDevice->mFence.get(), fence));
    mFenceValue++;
   
    // Wait until the previous frame is finished.
    if (mDevice->mFence->GetCompletedValue() < fence) {
      breakOnFail(mDevice->mFence->SetEventOnCompletion(fence, mDevice->mFenceEvent));
      WaitForSingleObject(mDevice->mFenceEvent, INFINITE);
    }
   
    mDevice->getOutput()->syncFrameIndex();

  // mDevice->waitForFence(mFenceValue);
}

#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Application/Window.hpp"

struct DeviceData {
  IDXGIFactory4Ptr dxgiFactory = nullptr;
  IDXGISwapChain3Ptr swapChain = nullptr;
  bool isWindowIncluded = false;
};

void d3dTraceHR(const std::string& msg, HRESULT hr) {
  char hr_msg[512];
  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr, 0, hr_msg, ARRAYSIZE(hr_msg), nullptr);

  std::string error_msg = msg + ".\nError! " + hr_msg;
  ERROR_AND_DIE(error_msg);
}

void getHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1*& ppAdapter) {
  IDXGIAdapter1* adapter = nullptr;
  ppAdapter = nullptr;

  for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex) {
    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);

    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      // Don't select the Basic Render Driver adapter.
      continue;
    }

    // Check to see if the adapter supports Direct3D 12, but don't create the
    // actual device yet.
    if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
      break;
    }
  }

  ppAdapter = adapter;
}

eCommandQueueType asCommandQueueType(RHIContextData::CommandQueueType type) {
  switch (type) {
    case RHIContextData::CommandQueueType::Copy:
      return D3D12_COMMAND_LIST_TYPE_COPY;
    case RHIContextData::CommandQueueType::Compute:
      return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case RHIContextData::CommandQueueType::Direct:
      return D3D12_COMMAND_LIST_TYPE_DIRECT;
    default:
      ENSURES(false);
      return D3D12_COMMAND_LIST_TYPE_DIRECT;
  }
}

void RHIDevice::cleanup() {

  mRenderContext->flush();

  mGpuDescriptorPool.reset();
  mCpuDescriptorPool.reset();
  mRenderContext.reset();
  SAFE_DELETE(mDeviceData);

  for (auto& b : mBackBuffers) {
    b.reset();
  }

  for (auto& b : mDepthBuffer) {
    b.reset();
  }

  while (!mDeferredRelease.empty()) {
    mDeferredRelease.pop();
  }

  mFrameFence.reset();
}


bool RHIDevice::rhiInit() {
  DeviceData* data = new DeviceData;

  mDeviceData = data;

  uint dxgiFlags = 0;

#if defined(_DEBUG)
  // enable debug layer for debug mode.
  // have to do this step before create device or it will inavalidate the active device
  ID3D12Debug* debugLayer;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
    debugLayer->EnableDebugLayer();

    dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
  }
#endif

  d3d_call(CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&mDeviceData->dxgiFactory)));


  IDXGIAdapter1* hardwareAdapter = nullptr;
  getHardwareAdapter(data->dxgiFactory.Get(), hardwareAdapter);

  // no hardware support for dx12 then use a wrap device
  if (hardwareAdapter == nullptr) {
    IDXGIAdapter* warpAdapter;
    d3d_call(data->dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

    d3d_call(D3D12CreateDevice(
      warpAdapter,
      D3D_FEATURE_LEVEL_11_0,
      IID_PPV_ARGS(&mRhiHandle)
    ));
  } else {
    d3d_call(D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mRhiHandle)));
  }

  if(mRhiHandle == nullptr) {
    return false;
  }

  /*
   *  for (uint32_t i = 0; i < RHIContextData::CommandQueueType::NUM_COMMAND_QUEUE_TYPE; i++)
        {
            for (uint32_t j = 0; j < desc.cmdQueues[i]; j++)

      to create an array of queue
   */
  D3D12_COMMAND_QUEUE_DESC qDesc = {};
  qDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  qDesc.Type = asCommandQueueType(RHIContextData::CommandQueueType::Direct);

  d3d_call(mRhiHandle->CreateCommandQueue(&qDesc, IID_PPV_ARGS(&mCommandQueue)));

  u64 freq;
  mGpuTimestampFrequency = 1000.0 / (double)mCommandQueue->GetTimestampFrequency(&freq);

  mRenderContext = RHIContext::create(mCommandQueue);
  return createSwapChain();
}

bool RHIDevice::rhiPostInit() {

  DXGI_SWAP_CHAIN_DESC1 desc;
  mSwapChain->GetDesc1(&desc);
  for (uint i = 0; i < FRAME_COUNT; i++) {
    ID3D12Resource* res;
    mSwapChain->GetBuffer(i, IID_PPV_ARGS(&res));
    mBackBuffers[i] = Texture2::create(res);
    // mRenderContext->resourceBarrier(mBackBuffers[i].get(), RHIResource::State::RenderTarget);
    mDepthBuffer[i] = Texture2::create(desc.Width, desc.Height, TEXTURE_FORMAT_D24S8, RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::DepthStencil);
    // mRenderContext->resourceBarrier(mDepthBuffer[i].get(), RHIResource::State::RenderTarget);
  }

  mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
  mRenderContext->resourceBarrier(mBackBuffers[mCurrentBackBufferIndex].get(), RHIResource::State::RenderTarget);
  mRenderContext->resourceBarrier(mDepthBuffer[mCurrentBackBufferIndex].get(), RHIResource::State::DepthStencil);

  return true;
}

bool RHIDevice::createSwapChain() {
  //constexpr uint width = 1280, height = 720;

  //WNDCLASSEX windowClassDescription;
  //memset(&windowClassDescription, 0, sizeof(windowClassDescription));
  //windowClassDescription.cbSize = sizeof(windowClassDescription);
  //windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
  //windowClassDescription.lpfnWndProc = windowProc;
  //// Register our Windows message-handling function
  //windowClassDescription.hInstance = GetModuleHandle(NULL);
  //windowClassDescription.hIcon = NULL;
  //windowClassDescription.hCursor = NULL;
  //windowClassDescription.lpszClassName = L"dx12test";
  //RegisterClassEx(&windowClassDescription);

  //mWindow = CreateWindowEx(
  //  WS_EX_APPWINDOW,
  //  L"dx12test",
  //  L"dx12test",
  //  WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED,
  //  CW_USEDEFAULT,
  //  CW_USEDEFAULT,
  //  width,
  //  height,
  //  NULL,
  //  NULL,
  //  GetModuleHandle(NULL),
  //  NULL);
  //ShowWindow(mWindow, SW_SHOW);


  IDXGISwapChain1* sc;

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.BufferCount = FRAME_COUNT; // front buffer & back buffer
  swapChainDesc.Width = 0;
  swapChainDesc.Height = 0; // will get figured out and fit the window, when calling `CreateSwapChainForHwnd`
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // https://msdn.microsoft.com/en-us/library/hh706346(v=vs.85).aspx
  swapChainDesc.SampleDesc.Count = 1;

  d3d_call(
    mDeviceData->dxgiFactory->CreateSwapChainForHwnd(
    mCommandQueue.Get(),
    mWindow, &swapChainDesc, nullptr, nullptr, &sc
  ));

  sc->QueryInterface(IID_PPV_ARGS(&mDeviceData->swapChain));
  mSwapChain = (IDXGISwapChain3*)sc;
  // do not support fullscreen 
  d3d_call(mDeviceData->dxgiFactory->MakeWindowAssociation(
    mWindow, DXGI_MWA_NO_ALT_ENTER));

  return true;
}

void RHIDevice::executeDeferredRelease() {
  // mpResourceAllocator->executeDeferredReleases();

  u64 gpuVal = mFrameFence->gpuVaule();

  while(!mDeferredRelease.empty() && mDeferredRelease.front().frame <= gpuVal) {
    mDeferredRelease.pop();
  }

  mCpuDescriptorPool->executeDeferredRelease();
  mGpuDescriptorPool->executeDeferredRelease();
}

void RHIDevice::present() {
  mRenderContext->resourceBarrier(backBuffer().get(), RHIResource::State::Present);
  mRenderContext->flush();
  mFrameFence->gpuSignal(mRenderContext->mContextData->commandQueue());
  d3d_call(mSwapChain->Present(1, 0));
  executeDeferredRelease();

  mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
  mRenderContext->resourceBarrier(backBuffer().get(), RHIResource::State::RenderTarget);
  mRenderContext->resourceBarrier(depthBuffer().get(), RHIResource::State::DepthStencil);
}

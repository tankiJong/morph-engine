#include "Dx12Output.hpp"
#include "d3d12.h"
#include <dxgi1_4.h>
#include "dx12util.hpp"
#include "Dx12Device.hpp"
#include "Dx12Instance.hpp"

bool gQuit = false;
LRESULT CALLBACK windowProc(HWND window, uint message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_DESTROY:
    case WM_CLOSE:
    {
      gQuit = true;
      break;; // "Consumes" this message (tells Windows "okay, we handled it")
    }
    default:

      break;
  }
  return ::DefWindowProc(window, message, wparam, lparam);
}



void WindowsMessageHandlingProcedure(uint wmMessageCode, size_t /*wParam*/, size_t lParam) {
  switch (wmMessageCode) {
    // App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4

  }
}

void Dx12Output::syncFrameIndex() {
  mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}

ID3D12Resource* Dx12Output::getCurrentBackBuffer() {
  return mRenderTargets[mFrameIndex].get();
}

uint Dx12Output::getFrameIndex() {
  return mFrameIndex;
}

void Dx12Output::present() {
  mSwapChain->Present(1, 0);
}

bool Dx12Output::quit() {
  return gQuit;
}

Dx12Output::Dx12Output(Dx12Device* device) {
  mDevice.reset(device);

  constexpr uint width = 1280, height = 720;

  WNDCLASSEX windowClassDescription;
  memset(&windowClassDescription, 0, sizeof(windowClassDescription));
  windowClassDescription.cbSize = sizeof(windowClassDescription);
  windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
  windowClassDescription.lpfnWndProc = windowProc;
  // Register our Windows message-handling function
  windowClassDescription.hInstance = GetModuleHandle(NULL);
  windowClassDescription.hIcon = NULL;
  windowClassDescription.hCursor = NULL;
  windowClassDescription.lpszClassName = L"dx12test";
  RegisterClassEx(&windowClassDescription);

  mWindow = CreateWindowEx(
    WS_EX_APPWINDOW,
    L"dx12test",
    L"dx12test",
    WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    width,
    height,
    NULL,
    NULL,
    GetModuleHandle(NULL),
    NULL);
  ShowWindow(mWindow, SW_SHOW);

  mViewport.TopLeftX = 0;
  mViewport.TopLeftY = 0;
  mViewport.Width = width;
  mViewport.Height = height;
  mViewport.MinDepth = D3D12_MIN_DEPTH;
  mViewport.MaxDepth = D3D12_MAX_DEPTH;

  mScissorRect.left = 0;
  mScissorRect.top = 0;
  mScissorRect.right = width;
  mScissorRect.bottom = height;


  IDXGISwapChain1* sc;

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.BufferCount = FRAME_COUNT; // front buffer & back buffer
  swapChainDesc.Width = 0;
  swapChainDesc.Height = 0; // will get figured out and fit the window, when calling `CreateSwapChainForHwnd`
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // https://msdn.microsoft.com/en-us/library/hh706346(v=vs.85).aspx
  swapChainDesc.SampleDesc.Count = 1;

  breakOnFail(
    device->mInstance->mGIFractory->CreateSwapChainForHwnd(
      device->mCommandQueue.get(), 
      mWindow, &swapChainDesc, nullptr, nullptr, &sc
  ));

  mSwapChain.reset((IDXGISwapChain3*)sc);
  // do not support fullscreen 
  breakOnFail(device->mInstance->mGIFractory->MakeWindowAssociation(
    mWindow, DXGI_MWA_NO_ALT_ENTER));

  mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

  {
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
      device->mRtvHeap->GetCPUDescriptorHandleForHeapStart());

    ID3D12Device* nativeDevice = (ID3D12Device*)mDevice->nativeDevice();
    // Create a RTV for each frame.
    for (UINT n = 0; n < FRAME_COUNT; n++) {
      ID3D12Resource* rt = mRenderTargets[n].get();
      breakOnFail(mSwapChain->GetBuffer(n, IID_PPV_ARGS(&rt)));
      mRenderTargets[n].reset(rt);
      nativeDevice->CreateRenderTargetView(rt, nullptr, rtvHandle);
      rtvHandle.ptr += 1 * mDevice->mRtvDexcriptorSize;
    }
  }
}


#include "Dx12Instance.hpp"
#include "d3d12.h"
#include <dxgi1_4.h>

#include "dx12util.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12Device.hpp"

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


S<Dx12Device> Dx12Instance::createDevice() {
  UINT dxgiFactoryFlags = 0;
  ID3D12Device* device = nullptr;

  /****************************************
  *
  * enable debug layer if it's in debug mode
  *
  ****************************************/
#if defined(_DEBUG)
  // enable debug layer for debug mode.
  // have to do this step before create device or it will inavalidate the active device
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugLayer)))) {
    mDebugLayer->EnableDebugLayer();

    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
  }
#endif

  /****************************************
  *
  * create render device
  *
  ****************************************/
  IDXGIFactory4* factory;
  breakOnFail(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
  mGIFractory = factory;

  IDXGIAdapter1* hardwareAdapter = nullptr;
  getHardwareAdapter(factory, hardwareAdapter);

  // no hardware support for dx12 then use a wrap device
  if (hardwareAdapter == nullptr) {
    IDXGIAdapter* warpAdapter;
    breakOnFail(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

    breakOnFail(D3D12CreateDevice(
      warpAdapter,
      D3D_FEATURE_LEVEL_11_0,
      IID_PPV_ARGS(&device)
    ));
  } else {
    breakOnFail(D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
  }

  EXPECTS(device != nullptr);

  S<Dx12Device> dxDevice;
  dxDevice.reset(new Dx12Device(this, device));
  return dxDevice;
}


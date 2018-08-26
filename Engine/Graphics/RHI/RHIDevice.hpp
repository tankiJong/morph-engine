#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/DescriptorPool.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"

class RHIContext;
struct DeviceData;

class RHIDevice: std::enable_shared_from_this<RHIDevice> {
  friend class RHIContext; // temp
public:
  static constexpr uint FRAME_COUNT = 2;
  using sptr_t = S<RHIDevice>;
  using scptr_t = S<const RHIDevice>;
  using rhi_handle_t = device_handle_t;
  bool init();
  RHIContext::sptr_t defaultRenderContext() const { return mRenderContext; }

  void releaseResource(rhi_obj_handle_t res);

  // excute the command list
  void flush();

  device_handle_t nativeDevice() const { return mRhiHandle; }

  void present();

  DescriptorPool::sptr_t cpuDescriptorPool() { return mCpuDescriptorPool; };
  DescriptorPool::sptr_t gpuDescriptorPool() { return mGpuDescriptorPool; }

  Texture2::sptr_t& backBuffer();
  Texture2::sptr_t& depthBuffer();

  ~RHIDevice();
  void cleanup();
  static sptr_t create(window_handle_t winHandle);

  static sptr_t get();
  window_handle_t        mWindow;
protected:
  struct res_release {
    size_t frame;
    rhi_obj_handle_t res;
  };

  bool rhiInit();
  bool rhiPostInit();
  bool createSwapChain();
  void executeDeferredRelease();
  std::queue<res_release> mDeferredRelease;
  DescriptorPool::sptr_t mGpuDescriptorPool;
  DescriptorPool::sptr_t mCpuDescriptorPool;
  RHIContext::sptr_t     mRenderContext;
  Fence::sptr_t          mFrameFence;
  DeviceData*            mDeviceData;
  rhi_handle_t           mRhiHandle;
  command_queue_handle_t mCommandQueue;
  swapchain_handle_t     mSwapChain;
  Texture2::sptr_t       mBackBuffers[FRAME_COUNT];
  Texture2::sptr_t       mDepthBuffer[FRAME_COUNT];
  double                 mGpuTimestampFrequency;
  bool mPendingDelete = false;
  uint mCurrentBackBufferIndex = 0;
};


class __SCOPED_GPU_EVENT {
public:
  __SCOPED_GPU_EVENT(const char* name) {
    RHIDevice::get()->defaultRenderContext()->beginEvent(name);
  }
  ~__SCOPED_GPU_EVENT() {
    RHIDevice::get()->defaultRenderContext()->endEvent();
  }
};
#define SCOPED_GPU_EVENT(name) __SCOPED_GPU_EVENT APPEND(__GPU_EVENT, __LINE__)(name)
#define GPU_FUNCTION_EVENT() SCOPED_GPU_EVENT(__FUNCTION__)
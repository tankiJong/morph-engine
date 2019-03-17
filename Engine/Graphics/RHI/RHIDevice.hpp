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

  const Texture2::sptr_t& backBuffer() const;
  const Texture2::sptr_t& depthBuffer() const;

  ~RHIDevice();
  void cleanup();
  static sptr_t create(window_handle_t winHandle);

  static const sptr_t& get();
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
  bool                   mAllowTearing = false;
  bool mPendingDelete = false;
  uint mCurrentBackBufferIndex = 0;
};


class __SCOPED_GPU_EVENT {
public:
  __SCOPED_GPU_EVENT(RHIContext& c, const char* name) {
    ctx = &c;
    ctx->beginEvent(name);
  }
  __SCOPED_GPU_EVENT(const char* name) {
    ctx = RHIDevice::get()->defaultRenderContext().get();
    ctx->beginEvent(name);
  }
  ~__SCOPED_GPU_EVENT() {
    ctx->endEvent();
  }

  RHIContext* ctx = nullptr;
};

#define SCOPED_GPU_EVENT(ctx, name) __SCOPED_GPU_EVENT APPEND(__GPU_EVENT, __LINE__)(ctx, name)
#define GPU_FUNCTION_EVENT(ctx) SCOPED_GPU_EVENT(ctx, __FUNCTION__)
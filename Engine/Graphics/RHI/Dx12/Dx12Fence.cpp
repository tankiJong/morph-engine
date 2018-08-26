#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/Fence.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

struct fence_data_t {
  HANDLE eventHandle = INVALID_HANDLE_VALUE;
};

Fence::~Fence() {
  CloseHandle(mData->eventHandle);
  SAFE_DELETE(mData);
}

Fence::sptr_t Fence::create() {
  sptr_t fence = sptr_t(new Fence());
  fence->mData = new fence_data_t;
  fence->mData->eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);

  auto device = RHIDevice::get()->nativeDevice();
  d3d_call(device->CreateFence(fence->mCpuValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence->mHandle)));
  fence->mCpuValue++;

  return fence;
}

u64 Fence::gpuVaule() const {
  return mHandle->GetCompletedValue();
}

u64 Fence::gpuSignal(command_queue_handle_t cq) {
  d3d_call(cq->Signal(mHandle.Get(), mCpuValue));
  mCpuValue++;
  return mCpuValue - 1;
}

void Fence::syncCpu() {
  uint64_t gpuVal = gpuVaule();
  if (gpuVal < mCpuValue - 1) {
    d3d_call(mHandle->SetEventOnCompletion(mCpuValue - 1, mData->eventHandle));
    WaitForSingleObject(mData->eventHandle, INFINITE);
  }
}

void Fence::syncGpu(command_queue_handle_t cq) {
  d3d_call(cq->Wait(mHandle.Get(), mCpuValue - 1));
}

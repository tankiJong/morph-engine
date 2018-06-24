#pragma once
#include "RHI.hpp"
#include "Engine/Core/common.hpp"

struct fence_data_t;

class Fence: public std::enable_shared_from_this<Fence> {
public:
  using handle_t = fence_handle_t;
  using sptr_t = S<Fence>;
  using scptr_t = S<const Fence>;

  ~Fence();
  u64 cpuValue() const { return mCpuValue; }

  static sptr_t create();

  u64 gpuVaule() const;
  u64 gpuSignal(command_queue_handle_t cq);
  void syncCpu();
  void syncGpu(command_queue_handle_t cq);
protected:
  Fence(): mCpuValue(0) {}
  u64 mCpuValue;

  handle_t mHandle;
  fence_data_t* mData = nullptr;
};

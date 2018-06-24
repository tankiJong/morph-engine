#pragma once
#include "RHI.hpp"
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/FencedPool.hpp"

struct RHIMetaData;
class RHIContextData: public std::enable_shared_from_this<RHIContextData> {
public:
  using sptr_t = S<RHIContextData>;
  using scptr_t = S<const RHIContextData>;

  enum class CommandQueueType {
    Copy,
    Compute,
    Direct,
    NUM_COMMAND_QUEUE_TYPE,
  };

  ~RHIContextData();
  static sptr_t create(CommandQueueType type, command_queue_handle_t handle);

  RHIMetaData* metaData() const { return mMetaData; }
  Fence::sptr_t fence() const { return mFence; }
  command_list_handle_t commandList() const { return mList; }
  command_queue_handle_t commandQueue() const { return mQueue; }
  virtual void flush();
protected:
  RHIMetaData* mMetaData;
  CommandQueueType mType;
  command_list_handle_t mList;
  command_queue_handle_t mQueue;
  Fence::sptr_t mFence;
  command_alloctor_handle_t mAllocator;
};

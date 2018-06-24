#include "Engine/Graphics/RHI/RHIContextData.hpp"
#include "Engine/Graphics/RHI/Dx12/dx12util.hpp"
#include "Engine/Graphics/RHI/Dx12/Dx12MetaData.h"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

template<D3D12_COMMAND_LIST_TYPE type>
static ID3D12CommandAllocatorPtr aquireCommandAlloctor(void* pUserData) {
  ID3D12CommandAllocatorPtr alloc;
  d3d_call(RHIDevice::get()->nativeDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&alloc)));

  return alloc;
}

eCommandQueueType asRhiCommandQueue(RHIContextData::CommandQueueType type) {
  switch (type) {
    case RHIContextData::CommandQueueType::Copy:
      return D3D12_COMMAND_LIST_TYPE_COPY;
    case RHIContextData::CommandQueueType::Compute:
      return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case RHIContextData::CommandQueueType::Direct:
      return D3D12_COMMAND_LIST_TYPE_DIRECT;
    default:
      ERROR_AND_DIE("should not reach here");
      return D3D12_COMMAND_LIST_TYPE_DIRECT;
  }
}

RHIContextData::~RHIContextData() {}


RHIContextData::sptr_t RHIContextData::create(CommandQueueType type, command_queue_handle_t handle) {
  sptr_t ctxData = sptr_t(new RHIContextData());

  ctxData->mFence = Fence::create();
  ctxData->mQueue = handle;
  ctxData->mMetaData = new RHIMetaData();

  D3D12_COMMAND_LIST_TYPE tt = asRhiCommandQueue(type);

  switch(tt) {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
      ctxData->mMetaData->allocatorPool
        = FencedPool<command_alloctor_handle_t>
        ::create(ctxData->mFence, aquireCommandAlloctor<D3D12_COMMAND_LIST_TYPE_DIRECT>);
    break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
      ctxData->mMetaData->allocatorPool
        = FencedPool<command_alloctor_handle_t>
        ::create(ctxData->mFence, aquireCommandAlloctor<D3D12_COMMAND_LIST_TYPE_COMPUTE>);
      break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
      ctxData->mMetaData->allocatorPool
        = FencedPool<command_alloctor_handle_t>
        ::create(ctxData->mFence, aquireCommandAlloctor<D3D12_COMMAND_LIST_TYPE_COPY>);
      break;
    default:
      ERROR_AND_DIE("should not be here");
  }

  ctxData->mAllocator = ctxData->mMetaData->allocatorPool->aquire();
  
  d3d_call(RHIDevice::get()->nativeDevice()
            ->CreateCommandList(0, tt, ctxData->mAllocator, 
                                nullptr, IID_PPV_ARGS(&ctxData->mList)));

  return ctxData;
}

void RHIContextData::flush() {
  d3d_call(mList->Close());
  ID3D12CommandList* list = mList.GetInterfacePtr();
  mQueue->ExecuteCommandLists(1, &list);
  mFence->gpuSignal(mQueue);
  // Memory leak here, because fence is not managed, so never get chance to release alloc
  mAllocator = mMetaData->allocatorPool->aquire();
  d3d_call(mAllocator->Reset());
  d3d_call(mList->Reset(mAllocator, nullptr));
}

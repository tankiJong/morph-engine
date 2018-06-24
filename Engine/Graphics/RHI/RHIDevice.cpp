#include "RHI.hpp"
#include "RHIDevice.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
RHIDevice::sptr_t gDevice;


RHIDevice::sptr_t RHIDevice::create() {
  if(gDevice) {
    ERROR_AND_DIE("only support one device");
  }
  gDevice = sptr_t(new RHIDevice);

  if(gDevice->init() == false) {
    gDevice = nullptr;
  }

  return gDevice;
}

RHIDevice::sptr_t RHIDevice::get() {
  return gDevice;
}


bool RHIDevice::init() {
  // from Falcor, not sure the logic, so put it here for now.
  // const uint32_t kDirectQueueIndex = (uint32_t)LowLevelContextData::CommandQueueType::Direct;
  // assert(desc.cmdQueues[kDirectQueueIndex] > 0);

  if (rhiInit() == false) return false;

  DescriptorPool::Desc poolDesc;

  // Falcor Comment: DX12 guarantees at least 1,000,000 descriptors

  poolDesc
    .setDescCount(DescriptorPool::Type::TextureSrv, 1e4)
    .setDescCount(DescriptorPool::Type::Sampler, 2048)
    .setShaderVisibility(true);

  poolDesc
    .setDescCount(DescriptorPool::Type::Cbv, 16 * 1024)
    .setDescCount(DescriptorPool::Type::TextureUav, 16 * 1024);

  poolDesc
    .setDescCount(DescriptorPool::Type::StructuredBufferSrv, 2 * 1024)
    .setDescCount(DescriptorPool::Type::StructuredBufferUav, 2 * 1024)
    .setDescCount(DescriptorPool::Type::TypedBufferSrv, 2 * 1024)
    .setDescCount(DescriptorPool::Type::TypedBufferUav, 2 * 1024);

  mGpuDescriptorPool = DescriptorPool::create(poolDesc, mRenderContext->contextData()->fence());

  poolDesc
    .setShaderVisibility(false)
    .setDescCount(DescriptorPool::Type::Rtv, 16 * 1024)
    .setDescCount(DescriptorPool::Type::Dsv, 1024);

  mCpuDescriptorPool = DescriptorPool::create(poolDesc, mRenderContext->contextData()->fence());

  mRenderContext->flush();

  // later may be also init resource allocator for better performance,
  // all rhiRes will allocate from Allocator instead of Directly from RHI

  mFrameFence = Fence::create();

  // init/update fbo happens here, see updateDefaultFBO(width, height)
  return true;
}

void RHIDevice::releaseResource(rhi_obj_handle_t res) {
  TODO("properly release resource");
}

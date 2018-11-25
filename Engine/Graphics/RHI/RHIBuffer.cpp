#include "RHIBuffer.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

RHIBuffer::~RHIBuffer() {
  if(RHIDevice::get())
    RHIDevice::get()->releaseResource(mRhiHandle);
}

RHIBuffer::sptr_t RHIBuffer::create(
  size_t size, RHIResource::BindingFlag binding, RHIBuffer::CPUAccess cpuAccess, const void* data) {

  RHIBuffer::sptr_t buffer = sptr_t(new RHIBuffer(size, binding, cpuAccess));

  if(buffer->rhiInit(data != nullptr)) {
    if (data) buffer->updateData(data, 0, size);
    return buffer;
  }

  ERROR_AND_DIE("fail to create buffer");
}

void RHIBuffer:: updateData(const void* data, size_t offset, size_t size) {
  if(mCpuAccess == CPUAccess::Write) {
    byte_t* dst = (byte_t*)map(MapType::WriteDiscard) + offset;
    std::memcpy(dst, data, size);
    unmap();
  } else {
    RHIDevice::get()->defaultRenderContext()->updateBuffer(this, data, offset, size);
  }
}



#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"

class RHIBuffer: public RHIResource, public inherit_shared_from_this<RHIResource, RHIBuffer> {
public:
  using sptr_t = S<RHIBuffer>;
  using scptr_t = S<const RHIBuffer>;
  using inherit_shared_from_this<RHIResource, RHIBuffer>::shared_from_this;

  enum class CPUAccess {
    None,    ///< The CPU can't access the buffer's content. The buffer can be updated using Buffer#updateData()
    Write,   ///< The buffer can be mapped for CPU writes
    Read,    ///< The buffer can be mapped for CPU reads
  };

  enum class GPUAccess {
    ReadOnly = 0,  ///< Buffer will mapped for GPU read only.
    ReadWrite = 1, ///< Buffer will mapped for GPU read-write.
    WriteOnly = 2, ///< Buffer will mapped for GPU write only.
  };

  enum class MapType {
    Read,           ///< Map the buffer for read access. Buffer had to be created with AccessFlags#MapWrite flag.
    WriteDiscard,   ///< Map the buffer for write access, discarding the previous content of the entire buffer. Buffer had to be created with AccessFlags#MapWrite flag.
  };

  ~RHIBuffer();

  // this function will live on RHIDevice, just for logical thinking, so write down
  static RHIBuffer::sptr_t create(size_t size, RHIResource::BindingFlag binding, RHIBuffer::CPUAccess cpuAccess, const void* data = nullptr);

  void updateData(const void* data, size_t offset, size_t size);

  template<typename T>
  void updateData(const T& data, size_t offset = 0) {
    static_assert(!std::is_pointer_v<T>);
    updateData(&data, offset, sizeof(T));
  }

  // for now, I do not use allocator yet, so every buffer will be the brand new whole buffer 
  u64 gpuAddressOffset() const { return 0; };

  u64 gpuAddress() const;

  size_t size() const { return mSize; }

  void* map(MapType type);

  void unmap();

  inline const CPUAccess cpuAccess() const { return mCpuAccess; }

  virtual const ConstantBufferView* cbv() override;
  virtual const UnorderedAccessView* uav() override;

protected:
  bool rhiInit(bool hasInitData);

  RHIBuffer(size_t size, BindingFlag binding, CPUAccess update)
  : RHIResource(Type::Buffer, binding), mSize(size), mCpuAccess(update) {}

  size_t mSize= 0;
  CPUAccess mCpuAccess;

};

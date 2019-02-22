#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/File/Blob.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"

/*
 * Array of same type elements
 */
class TypedBuffer : public RHIBuffer {
public:
  class sptr_t: public S<TypedBuffer> {
  public:
    sptr_t(): S<TypedBuffer>() {}
    sptr_t(TypedBuffer* buffer): S<TypedBuffer>(buffer) {}

    class Element {
      Element(TypedBuffer* buffer, u32 index)
        : mBuffer(buffer), mIndex(index) {}

      template<typename T>
      void operator=(const T& val) { mBuffer->set(val, mIndex); }

      template<typename T>
      operator T() const { return mBuffer->get(mIndex); }

    protected:
      TypedBuffer* mBuffer = nullptr;
      u32 mIndex;

    };

    template<typename T>
    T& operator[](u32 index) {
      EXPECTS(index * sizeof(T) < get()->mData.size());
      return *(get()->mData.as<T*>() + index);
    }
  };

  void uploadGpu();

  template<typename T>
  void set(u32 index, const T& val) {
    set(&val, sizeof(T), index * mStride);
  }

  template<typename T>
  void set(span<T> data) {
    mElementCount = (u32)data.size();
    mStride = sizeof(T);
    mData.set(data.data(), data.size() * sizeof(T));
    mCpuDirty = true;
  }

  void set(uint stride, uint count, const void* data);
  template<typename T>
  T& get(u32 index) {
    return *static_cast<T*>(get(index * mStride));
  }

  template<typename T>
  const T& get(u32 index) const {
    return *static_cast<const T*>(get(index * mStride));
  }

  const RHIBuffer& res() const {
    return *this;
  }

  u32 elementCount() const {
    return mElementCount;
  }

  u32 stride() const {
    return mStride;
  }

  virtual ShaderResourceView* srv(uint mipLevel = 0) const override;

  template<typename T>
  static sptr_t For(u32 eleCount = 1, BindingFlag bindingFlags = BindingFlag::ShaderResource) {
    return create(sizeof(T), eleCount, bindingFlags);
  }

  virtual const UnorderedAccessView* uav(uint mipLevel = 0) const override;

  const RHIBuffer& uavCounter() const { return *mUavCounter; }
  RHIBuffer& uavCounter() { return *mUavCounter; }

  static sptr_t create(u32 stride, u32 eleCount, BindingFlag bindingFlags = BindingFlag::ShaderResource);
protected:

  TypedBuffer(u32 eleCount, u32 stride, BindingFlag bindingFlags);

  void set(const void* data, u32 size, u32 byteOffset);
  void* get(u32 byteOffset);
  const void* get(u32 byteOffset) const;


  u32 mElementCount = 0;
  u32 mStride = 1;
  Blob mData;
  bool mCpuDirty = false;
  RHIBuffer::sptr_t mUavCounter;
  mutable ShaderResourceView::sptr_t mSrv;
};


using IndexBuffer = TypedBuffer;
using VertexBuffer = TypedBuffer;

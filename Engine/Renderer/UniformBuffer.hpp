#pragma once
#include "Engine/Core/common.hpp"
#include "RenderBuffer.hpp"
#include "type.h"
#include "Engine/File/Blob.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

class UniformBuffer {
public:
  UniformBuffer();
  ~UniformBuffer();

  void cleanup();

public:
  // copy data to the CPU and dirty the buffer
  void putCpu(size_t byteSize, void const *data);

  // update the gpu buffer from the local cpu buffer if dirty
  // and clears the dirty flag
  void putGpu();

  // sets the cpu and gpu buffers - clears the dirty flag
  void put(size_t byteSize, void const *data);

  // gets a pointer to the cpu buffer (const - so does not dirty)
  inline const void* get() const { return mData; };

  // get a mutable pointer to the cpu buffer.  Sets the dirty flag
  // as it expects the user to change it.
  void* get();

  // get this buffer's size in bytes
  inline size_t size() const { return mData.size(); };


  // templated helpers
public:
  //------------------------------------------------------------------------
  // let's me just set a structure, and it'll figure out the size
  template <typename T>
  void set(T const &v) {
    this->putCpu(sizeof(T), &v);
  }

  //------------------------------------------------------------------------
  // get's a constant reference to the CPU buffer as known struct 
  // would get the same as saying (T*) uniform_buffer->get_cpu_buffer(); 
  template <typename T>
  const T* as() const {
    return (const T*)this->get();
  }

  //------------------------------------------------------------------------
  template <typename T>
  T* as() {
    return (T*)this->get();
  }

public:
  inline uint handle() const { return mRenderBuffer.handle(); };

public:
  // render buffer backing this constant buffer (gpu memory
  RenderBuffer mRenderBuffer;
  // cpu copy of the data stored in m_render-buffer
  Blob mData;

  // uses to check if we should update gpu from cpu
  bool mDirtyBit = true;

public:
  //------------------------------------------------------------------------
  //
  template <typename T>
  static UniformBuffer* For(const T& v) {
    // Excercise - convenience constructor, 
    // will create a constant buffer for a specific struct, 
    // initializing to the structs contents; 
    UNIMPLEMENTED_RETURN(nullptr);

    UniformBuffer* buffer = new UniformBuffer();

    buffer->putCpu(sizeof(T), &v);

    return buffer;
    /* usage example
    time_buffer_t time_buffer;
    // fill time_buffer with data
    m_time_ubo = UniformBuffer::For( time_buffer );
    */
  }

};
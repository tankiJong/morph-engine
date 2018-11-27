#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/Endianness.hpp"


/*
 *            | -> write cursor
 * ------------------
 * xxxxxxxxxxx
 * ------------------
 *   | -> read cursor
 * 
 */

class BytePacker {
public:
  enum eStorageFlag: uint{
    STORAGE_OWN_BUFFER = BIT_FLAG(0),
    STORAGE_GROWABLE = BIT_FLAG(1),
  };

  enum eSeekDir {
    SEEK_DIR_BEGIN,
    SEEK_DIR_END,
    SEEK_DIR_CURRENT,
  };

  ~BytePacker();

  BytePacker(eEndianness byteOrder = ENDIANNESS_LITTLE);
  BytePacker(size_t size, eEndianness byteOrder = ENDIANNESS_LITTLE);
  BytePacker(size_t size, void* buffer, eEndianness byteOrder = ENDIANNESS_LITTLE);

  BytePacker(BytePacker&& mv);
  BytePacker& operator=(BytePacker&& rhs);

  BytePacker(const BytePacker&) = delete;
  BytePacker& operator=(const BytePacker&) = delete;

  void setEndianness(eEndianness e);

	// C4: write_array( data, count, stride )
	//     read_array( data, count, stride )  
	//     ... will give you the functionality of both write and append;

  // maybe flip data according to the endianness, update(forward) write cursor
  bool write(const void* data, size_t size);
  bool write(const char* data);
  bool write(size_t size);
  // just append raw data, do not process, update(forward) write cursor
  bool append(const void* data, size_t size);
  // just consume the data, do not process, update(forward) read cursor
  size_t consume(void* data, size_t size); // C4: Move this next to reads; 

  // extract data, convert endianness, update(forward) read cursor
  size_t read(void* outData, size_t maxRead);
  size_t read(char* data, size_t maxRead);
  size_t read(size_t& size);

  // buffer data size
  size_t size() const;
  size_t capacity() const;
  const void* data(size_t offset = 0) const;

  size_t tellr() const;
  size_t tellw() const;

  void clear();

  void seekr(intptr_t offset, eSeekDir dir = SEEK_DIR_BEGIN);
  void seekw(intptr_t offset, eSeekDir dir = SEEK_DIR_BEGIN);

protected:
  bool grow(size_t minSize);
  bool valid() const;

  eStorageFlag mFlag;
  eEndianness mByteOrder;
  span<byte_t> mBufferView;
  size_t mNextWrite = 0;
  size_t mNextRead = 0;
};

enum_class_operators(BytePacker::eStorageFlag);

template<typename T>
BytePacker& operator >>(BytePacker& lhs, T&& rhs) {
  lhs.read(&rhs, sizeof(T));
  return lhs;
}

template<typename T>
BytePacker& operator << (BytePacker& lhs, T&& rhs) {
  lhs.write((void*)&rhs, sizeof(T));
  return lhs;
}

template<typename T>
void operator >>(T&& lhs, BytePacker& rhs) {
  rhs.write(&lhs, sizeof(T));
}

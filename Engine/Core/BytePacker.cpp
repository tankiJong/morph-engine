#include "BytePacker.hpp"
#include <sstream>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include <vector>

// C4: Don't use "16" - use a define or const that is "DEFAULT_BUFFER_SIZE" or similar; 
BytePacker::BytePacker(eEndianness byteOrder)
  : mFlag(STORAGE_OWN_BUFFER | STORAGE_GROWABLE)
  , mByteOrder(byteOrder)
  , mBufferView(new byte_t[16], 16) {
}

BytePacker::BytePacker(size_t size, eEndianness byteOrder)
  : mFlag(STORAGE_OWN_BUFFER) 
  , mByteOrder(byteOrder)
  , mBufferView(new byte_t[size], size) {
}

BytePacker::BytePacker(size_t size, void* buffer, eEndianness byteOrder)
  : mFlag(eStorageFlag(0))
  , mByteOrder(byteOrder)
  , mBufferView((byte_t*)buffer, size) {
}

BytePacker::BytePacker(BytePacker&& mv) {
  mFlag = mv.mFlag;
  mByteOrder = mv.mByteOrder;
  mBufferView = mv.mBufferView;
  mNextWrite = mv.mNextWrite;
  mNextRead = mv.mNextRead;

  mv.mBufferView = span<byte_t>();
}

BytePacker& BytePacker::operator=(BytePacker&& rhs) {
  mFlag = rhs.mFlag;
  mByteOrder = rhs.mByteOrder;
  mBufferView = rhs.mBufferView;
  mNextWrite = rhs.mNextWrite;
  mNextRead = rhs.mNextRead;

  rhs.mBufferView = span<byte_t>();

  return *this;
}

BytePacker::~BytePacker() {
  if(is_set(mFlag, STORAGE_OWN_BUFFER)) {
    delete[] mBufferView.data();
  }
}

void BytePacker::setEndianness(eEndianness e) {
  mByteOrder = e;
}

bool BytePacker::write(const void* data, size_t size) {

  size_t nextWriteStamp = mNextWrite;

  bool re = append(data, size);

  if (!re) return false;

  // append already move the nextWrite, so move it back first

  // flip the data is endianness does not match
  toEndianness(mByteOrder, mBufferView.data() + nextWriteStamp, size);

  return true;
}

bool BytePacker::write(const char* data) {
  size_t size = strlen(data);

  bool re = write(size);
  re = re && append(data, size);

  return re;
}

bool BytePacker::write(size_t size) {
  // convert to my byte order, which everyone agree on.
	// C4: Do not convert!  Bit operators work WITH the endianness of the machine; 
  toEndianness(ENDIANNESS_LITTLE, size);

  
  size_t byteToWrite = 0;

	// C4: See changelist for what I did here -> see why it was equivalent; 
  uint8_t bytes[16];

  // the lower bit will be compressed first, so it will be pushed into buffer first

  while(size != 0) {
    uint8_t& b = bytes[byteToWrite];
    byteToWrite++;

    // xxxx xxxx,  [Flag: 1bit][data: 7bits from the input size]
    b = (size >> 7u) == 0 ? 0u : 1u;
    b <<= 7u;
    b = b | (size & 0x7f);

    size >>= 7u;
  }

  return append(bytes, byteToWrite);
}

bool BytePacker::append(const void* data, size_t size) {
  if (size + mNextWrite >= capacity()) {
    bool re = grow(size + mNextWrite);
    if (!re) {
      ERROR_RECOVERABLE("fail to write data into packer since there is not enough space and cannot grow");
      return re;
    }
  }

  memcpy(mBufferView.data() + mNextWrite, data, size);
  mNextWrite += size;
  return true;
}

size_t BytePacker::consume(void* data, size_t size) {
  size_t readCount = std::min(size, mNextWrite - mNextRead);

  memcpy(data, mBufferView.data() + mNextRead, readCount);
  mNextRead += size;
  return readCount;
}

size_t BytePacker::read(void* outData, size_t maxRead) {
  size_t readCount = consume(outData, maxRead);

  fromEndianness(mByteOrder, outData, readCount);

  return readCount;
}

size_t BytePacker::read(char* data, size_t maxRead) {
  size_t totalRead = 0;

  size_t stringLen;
  totalRead += read(stringLen);
  totalRead += stringLen;

  size_t cursorAfterString = mNextRead + stringLen;
  size_t readLen = stringLen;

  if(stringLen >= maxRead) {
    ERROR_RECOVERABLE("the buffer provided for the string is not big enough, so packer will only output part of the string to the buffer");
    readLen = maxRead;
  }

  consume(data, readLen);
  data[readLen] = 0;

  mNextRead = cursorAfterString;

  return totalRead;
}

size_t BytePacker::read(size_t& size) {
  size = 0;
  uint8_t byte;

  size_t index = 0;

  // the byte with smaller index are lower bits

  do {
    consume(&byte, 1);

    size |= size_t((byte & 0x7f)) << (index * 7u);

    index++;

  } while (byte & 0b10000000); // C4: 0x80


  ENSURES(index <= sizeof(size_t)+1);

  fromEndianness(ENDIANNESS_LITTLE, size);

  return index;
}

size_t BytePacker::size() const {
  return mNextWrite;
}

size_t BytePacker::capacity() const {
  return mBufferView.size();
}

const void* BytePacker::data(size_t offset) const {
  EXPECTS(offset < mBufferView.size());
  return mBufferView.data() + offset;
}

size_t BytePacker::tellr() const {
  return mNextRead;
}

size_t BytePacker::tellw() const {
  return  mNextWrite;
}

void BytePacker::clear() {
  mNextWrite = 0;
  mNextRead = 0;
}

// C4: Why int16_t?   64KB jumps seems uneeded; int or intptr_t
void BytePacker::seekr(int16_t offset, eSeekDir dir) {
  switch(dir) { 
    case SEEK_DIR_BEGIN:
      EXPECTS(offset >= 0);
      mNextRead = (size_t)offset;
    break;
    case SEEK_DIR_END:
      EXPECTS(offset >= 0);
      mNextRead = capacity() - offset;
    break;
    case SEEK_DIR_CURRENT:
      mNextRead = mNextRead + offset;
    break;
  }

  ENSURES(valid());
}

// C4: Second verse, same as the first...
void BytePacker::seekw(int16_t offset, eSeekDir dir) {
  switch (dir) {
    case SEEK_DIR_BEGIN:
      EXPECTS(offset > 0);
      mNextWrite = (size_t)offset;
      break;
    case SEEK_DIR_END:
      EXPECTS(offset > 0);
      mNextWrite = capacity() - offset;
      break;
    case SEEK_DIR_CURRENT:
      mNextWrite = mNextWrite + offset;
      break;
  }

  ENSURES(valid());
}

// is_set( flags, A | B ); 
// is_any_set( flags, A | B); 
// is_all_set( flags, A | B); // (flags & bits) == bits; 
// Does this return true if A, B, or both A & B?
// For more fun: Bit Twiddling Hacks: https://graphics.stanford.edu/~seander/bithacks.html

bool BytePacker::grow(size_t minSize) {
  if (!is_set(mFlag, STORAGE_GROWABLE)) return false;
  if (!is_set(mFlag, STORAGE_OWN_BUFFER)) return false;

  size_t currentSize = mBufferView.size();
  byte_t* src = mBufferView.data();

  size_t expectSize = std::max(currentSize * 2u, minSize);
  byte_t* dest = new byte_t[expectSize];

  memcpy(dest, src, currentSize);
  mBufferView = { dest, (int)expectSize };

  delete[] src;

  return true;
}

bool BytePacker::valid() const {
  std::vector<int> iv; // C4: Why?
  return mNextRead <= mNextWrite 
      && mNextWrite <= (size_t)mBufferView.size()
      && mBufferView.size() > 0;
}

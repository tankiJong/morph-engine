#include "NetPacket.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Net/UDPSession.hpp"
NetPacket::NetPacket()
  : BytePacker(NET_PACKET_MTU, mLocalBuffer, ENDIANNESS_LITTLE) {
  memset(mLocalBuffer, 0, NET_PACKET_MTU);
}

void NetPacket::fill(void* data, size_t size) {
  EXPECTS(size < NET_PACKET_MTU);

  memcpy(mLocalBuffer, data, size);
  seekw(size, SEEK_DIR_CURRENT);
}

void NetPacket::begin(uint8_t connectionIndex) {
  EXPECTS(mStampedMessage.empty());
  mStampedHeader.connectionIndex = connectionIndex;
  mStampUsableSize = NET_PACKET_MTU;
  clear();
}

void NetPacket::end() {
  mStampedHeader.unreliableCount = mStampedMessage.size();

  write(mStampedHeader);

  for(const NetMessage* msg: mStampedMessage) {
    write(*msg);
  }

  mStampedMessage.clear();
}

bool NetPacket::appendUnreliable(const NetMessage& msg) {
  size_t requireSize = 3 + msg.size();

  if (mStampUsableSize < requireSize) return false;

  mStampedMessage.push_back(&msg);
  mStampUsableSize -= requireSize;
  
  return true;
}

void NetPacket::read(header_t& header) {
  *this >> header.connectionIndex >> header.unreliableCount;
}

bool NetPacket::read(NetMessage& outMessage) {

  if (tellr() >= tellw()) return false;

  uint16_t total;
  uint8_t index;
  *this >> total >> index;

  void* buf = _alloca(total - 1u);

  size_t size = consume(buf, total - 1u);
  ENSURES(total - 1u == size);

  new(&outMessage) NetMessage(index);
  outMessage.append(buf, size);
  
  return true;
}

void NetPacket::write(const header_t& header) {
  *this << header.connectionIndex << header.unreliableCount;
}

bool NetPacket::write(const NetMessage& msg) {
  uint16_t total = (uint16_t)msg.size() + 1;
  uint8_t index = msg.index();

  *this << total << index;
  append(msg.data(), msg.size());

  return true;
}

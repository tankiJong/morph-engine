#include "NetPacket.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Net/UDPSession.hpp"


NetPacket::NetPacket()
  : BytePacker(NET_PACKET_MTU, mLocalBuffer, ENDIANNESS_LITTLE) {
  memset(mLocalBuffer, 0, NET_PACKET_MTU);
  mTimestamp = GetCurrentTimeSeconds();
}

NetPacket::NetPacket(const NetPacket& packet)
  : BytePacker(NET_PACKET_MTU, mLocalBuffer, ENDIANNESS_LITTLE) {
  memcpy(mLocalBuffer, packet.mLocalBuffer, NET_PACKET_MTU);
  mTimestamp = packet.mTimestamp;
}

void NetPacket::fill(const void* data, size_t size) {
  EXPECTS(size < NET_PACKET_MTU);

  memcpy(mLocalBuffer, data, size);
  seekw(size, SEEK_DIR_CURRENT);
}

void NetPacket::begin(const UDPConnection& connection) {
  EXPECTS(mStampedMessage.empty());
  mStampedHeader.connectionIndex = connection.owner()->selfIndex();
  mStampedHeader.ack = connection.nextAck();
  mStampedHeader.lastReceivedAck = connection.largestReceivedAck();
  mStampedHeader.previousReceivedAckBitField = connection.previousReceivedAckBitField();
  clear();
  mStampUsableSize = NET_PACKET_MTU;
  mStampUsableSize -= sizeof(mStampedHeader);
}

void NetPacket::end() {
  size_t size = mStampedMessage.size();

  ENSURES(size < 0xff);

  mStampedHeader.unreliableCount = (uint8_t)size;

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
  *this 
    >> header.connectionIndex
    >> header.ack
    >> header.lastReceivedAck
    >> header.previousReceivedAckBitField
    >> header.unreliableCount;
}

bool NetPacket::read(NetMessage& outMessage) {

  if (tellr() >= tellw()) return false;

  uint16_t total;
  uint8_t index;
  *this >> total;

  if (tellw() - tellr() < total) {
    return false;
  }
  *this >> index;

  void* buf = _alloca(total - 1u);

  size_t size = consume(buf, total - 1u);
  ENSURES(total - 1u == size);

  new(&outMessage) NetMessage(index);
  outMessage.append(buf, size);
  
  return true;
}

void NetPacket::receivedTime(double second) {
  mTimestamp = second;
}

void NetPacket::write(const header_t& header) {
  *this
    << header.connectionIndex
    << header.ack
    << header.lastReceivedAck
    << header.previousReceivedAckBitField
    << header.unreliableCount;
}

bool NetPacket::write(const NetMessage& msg) {
  bool reliable = msg.reliable();
  uint16_t total = (uint16_t)msg.size() + (reliable ? 3 : 1);
  uint8_t index = msg.index();
  *this << total << index;

  if(reliable) {
    uint16_t reliableId = msg.reliableId();
    *this << reliableId;
  }

  append(msg.data(), msg.size());

  return true;
}

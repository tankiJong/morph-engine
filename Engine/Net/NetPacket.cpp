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
  
  mStampedMessageReliable.clear();
  mStampedMessageUnreliable.clear();

  EXPECTS(mStampedMessageReliable.empty());
  EXPECTS(mStampedMessageUnreliable.empty());
  mStampedHeader.connectionIndex = connection.owner()->selfIndex();
  mStampedHeader.ack = connection.nextAck();
  mStampedHeader.lastReceivedAck = connection.largestReceivedAck();
  mStampedHeader.previousReceivedAckBitField = connection.previousReceivedAckBitField();
  clear();
  mStampUsableSize = NET_PACKET_MTU;
  mStampUsableSize -= sizeof(mStampedHeader);
}

void NetPacket::end() {
  size_t sizeReliable = mStampedMessageReliable.size();
  size_t sizeUnreliable = mStampedMessageUnreliable.size();

  ENSURES(sizeReliable + sizeUnreliable < 0xff);

  mStampedHeader.unreliableCount = (uint8_t)sizeUnreliable;
  mStampedHeader.reliableCount = (uint8_t)sizeReliable;
  mStampedHeader.messageCount = 
    mStampedHeader.unreliableCount + mStampedHeader.reliableCount;

  write(mStampedHeader);

  for(const NetMessage* msg: mStampedMessageReliable) {
    write(*msg);
  }
  for (const NetMessage* msg : mStampedMessageUnreliable) {
    write(*msg);
  }
}

bool NetPacket::append(NetMessage& msg) {
  size_t requireSize = 2 + msg.headerSize() + msg.size();

  if (mStampUsableSize < requireSize) return false;

  if(msg.reliable()) {
    if (mStampedMessageReliable.size() >= 
        UDPConnection::PacketTracker::MAX_RELIABLES_PER_PACKET) {
      return false;
    }
    mStampedMessageReliable.push_back(&msg);
    msg.lastSendSec() = GetCurrentTimeSeconds();
  } else {
    mStampedMessageUnreliable.push_back(&msg);
  }

  mStampUsableSize -= requireSize;
  
  return true;
}

void NetPacket::read(header_t& header) {
  *this
    >> header.connectionIndex
    >> header.ack
    >> header.lastReceivedAck
    >> header.previousReceivedAckBitField
    >> header.unreliableCount
    >> header.reliableCount
    >> header.messageCount;
}

bool NetPacket::read(NetMessage& outMessage, bool reliable) {

  if (tellr() >= tellw()) return false;

  uint16_t total;
  uint8_t index;

  *this >> total;

  if (tellw() - tellr() < total) {
    return false;
  }
  *this >> index;
  new(&outMessage) NetMessage(index);

  if(reliable) {
    uint16_t reliableId;
    *this >> reliableId;

    outMessage.reliableId(reliableId);
  }

  uint headerSize = NetMessage::headerSize(reliable);
  void* buf = _alloca(total - headerSize);

  size_t size = consume(buf, total - headerSize);
  ENSURES(total - headerSize == size);

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
    << header.unreliableCount
    << header.reliableCount
    << header.messageCount;
}

bool NetPacket::write(const NetMessage& msg) {
  bool reliable = msg.reliable();
  uint16_t total = (uint16_t)(msg.size() + msg.headerSize());
  uint8_t index = msg.index();
  *this << total << index;

  if(reliable) {
    uint16_t reliableId = msg.reliableId();
    *this << reliableId;
  }

  BytePacker::append(msg.data(), msg.size());

  return true;
}

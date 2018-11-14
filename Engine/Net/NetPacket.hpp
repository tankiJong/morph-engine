#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/BytePacker.hpp"
#include "Net.hpp"
#include <vector>
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/NetMessage.hpp"

class NetMessage;
class UDPConnection;

#define NET_PACKET_MTU (ETHERNET_MTU - 40 - 8) 
class NetPacket: protected BytePacker {
public:
  NetPacket();
  NetPacket(const NetPacket& packet);

  struct header_t {
    uint8_t connectionIndex = UINT8_MAX;

    uint16_t ack = 0;
    uint16_t lastReceivedAck;
    uint16_t previousReceivedAckBitField;

    uint8_t unreliableCount;
    uint8_t reliableCount;
    uint8_t messageCount;
  };

  static constexpr uint16_t INVALID_PACKET_ACK = 0xffffui16;
public:
  using BytePacker::data;
  using BytePacker::size;


  void fill(const void* data, size_t size);

  void begin(const UDPConnection& connection);
  void end();
  bool append(NetMessage& msg);

  void read(header_t& header);
  bool read(span<const NetMessage::Def> definitions, NetMessage& outMessage);

  void receivedTime(double second);

  double receivedTime() const { return mTimestamp; };

  uint16_t ack() const { return mStampedHeader.ack; }
public:
  bool operator==(const NetPacket& rhs) const {
    return mTimestamp == rhs.mTimestamp;
  };
  bool operator!=(const NetPacket& rhs) const {
    return !(*this == rhs);
  };
  bool operator>(const NetPacket& rhs) const {
    return mTimestamp > rhs.mTimestamp;
  };
  bool operator<(const NetPacket& rhs) const {
    return *this != rhs && !(*this > rhs);
  };

  const NetAddress& senderAddr() const { return mSenderAddr; }
  void senderAddr(const NetAddress& addr) {
    mSenderAddr = addr;
  }

  span<const NetMessage* const> messagesReliable() const { return mStampedMessageReliable; }

protected:
  void write(const header_t& header);
  bool write(const NetMessage& msg);

protected:
  byte_t mLocalBuffer[NET_PACKET_MTU];
  double mTimestamp;

  // temproal things
  std::vector<const NetMessage*> mStampedMessageUnreliable;
  std::vector<const NetMessage*> mStampedMessageReliable;
  size_t mStampUsableSize = NET_PACKET_MTU;
  header_t mStampedHeader;
  NetAddress mSenderAddr;
};

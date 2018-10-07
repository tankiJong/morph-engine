#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/BytePacker.hpp"
#include "Net.hpp"
#include <vector>
#include "Engine/Net/UDPConnection.hpp"
#include "Engine/Core/Time/Time.hpp"

class NetMessage;

#define NET_PACKET_MTU (ETHERNET_MTU - 40 - 8) 
class NetPacket: protected BytePacker {
public:
  NetPacket();
  NetPacket(const NetPacket& packet);

  struct header_t {
    uint8_t connectionIndex;
    uint8_t unreliableCount;
  };

public:
  using BytePacker::data;
  using BytePacker::size;


  void fill(const void* data, size_t size);

  void begin(uint8_t connectionIndex);
  void end();
  bool appendUnreliable(const NetMessage& msg);

  void read(header_t& header);
  bool read(NetMessage& outMessage);

  void receivedTime(double second);

  double receivedTime() const { return mTimestamp; };
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

protected:
  void write(const header_t& header);
  bool write(const NetMessage& msg);

protected:
  byte_t mLocalBuffer[NET_PACKET_MTU];
  double mTimestamp;

  // temproal things
  std::vector<const NetMessage*> mStampedMessage;
  size_t mStampUsableSize = NET_PACKET_MTU;
  header_t mStampedHeader;
  NetAddress mSenderAddr;
};

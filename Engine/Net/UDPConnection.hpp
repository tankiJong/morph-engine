#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include <vector>

class NetMessage;
class UDPSession;



class UDPConnection {
public:
  static constexpr double DEFAULT_HEARTBEAT_RATE = 5.0;

  bool valid() const { return mOwner != nullptr; }

  bool send(NetMessage& msg);
  void flush(bool force = false);
  const NetAddress& addr() const { return mAddress; };

  bool set(UDPSession& session, uint8_t index, const NetAddress& addr);

  void heartbeatFrequency(double freq);
  double tickFrequency(double freq);
protected:
  bool shouldSendPacket() const;
  bool tryAppendHeartbeat();
  std::vector<NetMessage> mOutboundUnreliables;
  NetAddress mAddress;
  UDPSession* mOwner = nullptr;
  uint8_t mIndexOfSession = uint8_t(-1);
  Interval mHeartBeatTimer{ DEFAULT_HEARTBEAT_RATE };
  double mLastSendSec = 0;
  double mTickSec = 0;
};


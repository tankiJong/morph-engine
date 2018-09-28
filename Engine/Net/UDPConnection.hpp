#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/NetAddress.hpp"
#include <vector>

class NetMessage;
class UDPSession;


class UDPConnection {
public:
  bool valid() const { return mOwner != nullptr; }

  bool send(const NetMessage& msg);
  void flush();
  const NetAddress& addr() const { return mAddress; };

  bool set(UDPSession& session, uint8_t index, const NetAddress& addr);
protected:
  bool shouldSendPacket();
  std::vector<NetMessage> mOutboundUnreliables;
  NetAddress mAddress;
  UDPSession* mOwner = nullptr;
  uint8_t mIndexOfSession = ~0;
};


#include "UDPConnection.hpp"
#include "Engine/Net/NetPacket.hpp"
#include "Engine/Net/UDPSession.hpp"

bool UDPConnection::send(const NetMessage& msg) {
  mOutboundUnreliables.push_back(msg);

  return true;
}

void UDPConnection::flush() {
  if (!shouldSendPacket()) return;

  NetPacket packet;

  packet.begin(mIndexOfSession);


  auto current = mOutboundUnreliables.cbegin();
  while(current != mOutboundUnreliables.cend()) {
    bool appended = 
      packet.appendUnreliable(*current);
    
    if(!appended) {
      break;
    }

    ++current;
  }

  packet.end();

  mOwner->send(mIndexOfSession, packet);

  mOutboundUnreliables.clear();
}

bool UDPConnection::set(UDPSession& session, uint8_t index, const NetAddress& addr) {
  mOwner = &session;
  mIndexOfSession = index;
  mAddress = addr;

  return true;
}

bool UDPConnection::shouldSendPacket() {
  return !mOutboundUnreliables.empty();
}

#include "UDPConnection.hpp"
#include "Engine/Net/NetPacket.hpp"
#include "Engine/Net/UDPSession.hpp"

bool UDPConnection::send(NetMessage& msg) {
  mOwner->finalize(msg);
  mOutboundUnreliables.push_back(msg);

  return true;
}

void UDPConnection::flush(bool force) {
  tryAppendHeartbeat();
  if(!force) {
    if (!shouldSendPacket()) return;
  }

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

  mLastSendSec = GetCurrentTimeSeconds();

  mOutboundUnreliables.clear();
}

bool UDPConnection::set(UDPSession& session, uint8_t index, const NetAddress& addr) {
  mOwner = &session;
  mIndexOfSession = index;
  mAddress = addr;
  mHeartBeatTimer.flush();
  return true;
}

void UDPConnection::heartbeatFrequency(double freq) {
  EXPECTS(freq != 0);
  mHeartBeatTimer.flush();
  mHeartBeatTimer.duration = 1.0 / freq;
}

double UDPConnection::tickFrequency(double freq) {
  mTickSec = 1.0 / freq;
  return mTickSec;
}

bool UDPConnection::shouldSendPacket() const {
  bool re = !mOutboundUnreliables.empty();

  bool canTick = 
    GetCurrentTimeSeconds() - mLastSendSec > 
    std::max(mOwner->tickSecond(), mTickSec);

  return re && canTick;
}

bool UDPConnection::tryAppendHeartbeat() {
  if (!mHeartBeatTimer.decrement()) return false;

  NetMessage heartbeat("heartbeat");
  send(heartbeat);

  return true;
}

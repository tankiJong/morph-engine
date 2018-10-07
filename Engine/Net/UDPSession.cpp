#include "UDPSession.hpp"
#include "UDPConnection.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Debug/Console/Command.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Net/NetPacket.hpp"
#include "Game/Gameplay/Map.hpp"

bool onHeartBeat(NetMessage, UDPSession::Sender&) {
  return true;
}

UDPSession::UDPSession() {
  
  // bind core message
  // * heartbeat
  on("heartbeat", onHeartBeat);

}

bool UDPSession::bind(uint16_t port) {
  bool re= mSock.bind(NetAddress::local(port));
  mSock.unsetOption(SOCKET_OPTION_BLOCKING);
  return re;
}

bool UDPSession::connect(uint8_t index, const NetAddress& addr) {
  if(mConnections.size() <= index) {
    mConnections.resize(index + 1);
  }

  return mConnections[index].set(*this, index, addr);
}

bool UDPSession::send(uint8_t index, NetMessage& msg) {
  EXPECTS(index < mConnections.size());

  if(!mConnections[index].valid()) {
    Log::tagf("net", "Fail to send message to an invalid connection [%u]", index);
    return false;
  }
  finalize(msg);
  return mConnections[index].send(msg);
}

bool UDPSession::in() {
  byte_t buf[NET_PACKET_MTU];
  NetAddress addr;

  while(true) {
    size_t size = mSock.receive(addr, buf, NET_PACKET_MTU);

    if(size == 0) {
      break;
    }

    NetPacket& packet = *allocPacket();
    packet.fill(buf, size);

    if(!verify(packet)) {
      Log::tagf("net", "Received invalid traffic from %s", addr.toString());
      return false;
    }

    // ----- sim ------
    if (checkRandomChance(mSimLossChance)) continue;

    double latency = getRandomf(
      (float)mMinSimLatencyMs, 
      (float)std::max(mMinSimLatencyMs, mMaxSimLatencyMs));

    packet.receivedTime(GetCurrentTimeSeconds() + latency * 0.001f);

    mPendingPackets.push(&packet);
  }

  processPackets();

  return true;
}

bool UDPSession::out() {
  for(UDPConnection& connection: mConnections) {
    if (!connection.valid()) continue;
    connection.flush();
  }

  return true;
}

bool UDPSession::send(uint8_t index, const NetPacket& packet) {
  size_t size = mSock.send(
    mConnections[index].addr(), 
    packet.data(), packet.size());

  return size > 0;
}

const UDPConnection* UDPSession::connection(uint8_t index) const {
  if(index >= mConnections.size()) return nullptr;
  return &mConnections[index];
}

UDPConnection* UDPSession::connection(uint8_t index) {
  if (index >= mConnections.size()) return nullptr;
  return &mConnections[index];
}


const UDPConnection* UDPSession::connection(const NetAddress& addr) const {
  return const_cast<UDPSession*>(this)->connection(addr);
}

UDPConnection* UDPSession::connection(const NetAddress& addr) {
  for(UDPConnection& connection: mConnections) {
    if(connection.addr() == addr) {
      return &connection;
    }
  }
  return nullptr;
}

bool UDPSession::verify(const NetPacket& packet) {
  NetPacket p;
  p.fill(packet.data(), packet.size());

  NetPacket::header_t header;

  p.read(header);

  for(uint8_t i = 0; i < header.unreliableCount; i++) {
    NetMessage msg;
    bool valid = p.read(msg);
    if (!valid) return false;
  }

  return true;
}

void UDPSession::simLatency(uint minMs, uint maxMs) {
  mMinSimLatencyMs = minMs;
  mMaxSimLatencyMs = maxMs;
}

void UDPSession::simLoss(float percentage) {
  if(percentage > 1.f) {
    Log::logf("fail to set illgial sim loss percentage: %.2f.", percentage);
    return;
  }
  mSimLossChance = percentage;
}

double UDPSession::tickSecond(double tickSec) {
  mTickSecond = tickSec;
  return mTickSecond;
}

double UDPSession::tickFrequency(float freq) {
  mTickSecond = 1.0 / freq;
  return mTickSecond;
}

double UDPSession::connectionTickFrequency(uint8_t index, float freq) {
  UDPConnection* conn = connection(index);
  if (conn == nullptr) {
    return false;
  }

  return conn->tickFrequency(freq); 
}

void UDPSession::heartbeatFrequency(float freq) {
  for(UDPConnection& connection: mConnections) {
    connection.heartbeatFrequency(freq);
  }
}

bool UDPSession::NetPacketComp::operator()(const NetPacket* lhs, const NetPacket* rhs) {
  return (*lhs) < (*rhs);
}

void UDPSession::finalize(NetMessage& msg) {

  uint8_t index = msg.index();

  if(index != NetMessage::Def::INVALID_MESSAGE_INDEX) {
    msg.setDefinition(mMessageDefs[index]);
    return;
  }

  ENSURES(msg.name().length() > 0);

  const std::string& name = msg.name();
  for(NetMessage::Def& def: mMessageDefs) {
    if(def.name == name) {
      msg.setDefinition(def);
      return;
    }
  }

  ERROR_AND_DIE("cannot find definition for the net message");
}

bool UDPSession::handle(NetMessage& msg, Sender& sender) {
  return mHandles.at(msg.name())(msg, sender);
}

void UDPSession::processPackets() {

  while(!mPendingPackets.empty()) {
    if (mPendingPackets.top()->
        receivedTime() > GetCurrentTimeSeconds()) break;
    NetPacket* packet = mPendingPackets.top();
    mPendingPackets.pop();

    NetPacket::header_t header;
    packet->read(header);

    for (uint i = 0; i < header.unreliableCount; i++) {
      NetMessage msg;
      packet->read(msg);
      finalize(msg);

      EXPECTS(msg.name().length() > 0);

      UDPConnection* conn = connection(header.connectionIndex);

      // do not handle traffic from unkown source
      if (conn == nullptr) {
        Log::log("receive unkown source traffic, throw away");
        continue;
      }

      Sender sender{ conn };

      handle(msg, sender);
    }

    
    freePacket(packet);
  }
}

NetPacket* UDPSession::allocPacket() {
  return new NetPacket();
}

void UDPSession::freePacket(NetPacket*& packet) {
  SAFE_DELETE(packet);
}

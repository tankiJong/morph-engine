#include "UDPSession.hpp"
#include "UDPConnection.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Debug/Console/Command.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Net/NetPacket.hpp"

bool UDPSession::bind(uint16_t port) {
  bool re= mSock.bind(NetAddress::local(port));
  mSock.unsetOption(SOCKET_OPTION_BLOCKING);
  return re;
}

bool UDPSession::connect(uint index, const NetAddress& addr) {
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

    NetPacket packet;
    packet.fill(buf, size);

    NetPacket::header_t header;
    packet.read(header);

    for(uint i = 0; i < header.unreliableCount; i++) {
      NetMessage msg;
      packet.read(msg);
      finalize(msg);
      handle(msg, addr);
    }
  }

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

const UDPConnection& UDPSession::connection(uint8_t index) const {
  EXPECTS(index < mConnections.size());
  return mConnections[index];
}

UDPConnection& UDPSession::connection(uint8_t index) {
  EXPECTS(index < mConnections.size());
  return mConnections[index];
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

bool UDPSession::handle(NetMessage& msg, const NetAddress& addr) {

  EXPECTS(msg.name().length() > 0);

  UDPConnection* conn = connection(addr);

  // do not handle traffic from unkown source
  if (conn == nullptr) return false;

  Sender sender{ conn };

  return mHandles.at(msg.name())(msg, sender);
}

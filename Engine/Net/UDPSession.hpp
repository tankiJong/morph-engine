#pragma once
#include "Engine/Core/common.hpp"
#include <functional>
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/UDPSocket.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Net/UDPConnection.hpp"

class NetMessage;
class NetPacket;

class UDPSession {
  friend class UDPConnection;
public:
  static constexpr uint8_t INVALID_CONNECTION_ID = ~0;

  struct MessageHandle {
    
  };

  struct Sender {
    UDPConnection* connection;
  };

  using message_handle_t = std::function<bool(NetMessage, Sender&)>;

  template<typename Func>
  MessageHandle on(const char* name, Func&& func) {
    mMessageDefs.push_back({ 0, name });

    std::sort(mMessageDefs.begin(), mMessageDefs.end(),
              [](NetMessage::Def& a, NetMessage::Def& b) {
      return a.name < b.name;
    });

    for(uint i = 0; i < mMessageDefs.size(); i++) {
      mMessageDefs[i].index = i;
    }

    mHandles[name] = func;
    return MessageHandle();
  }

  bool bind(uint16_t port);

  bool connect(uint index, const NetAddress& addr);

  bool send(uint8_t index, NetMessage& msg);

  bool in();

  bool out();

  bool send(uint8_t index, const NetPacket& packet);

  uint8_t selfIndex() const { return INVALID_CONNECTION_ID; }

  const UDPConnection& connection(uint8_t index) const;
  UDPConnection& connection(uint8_t index);

  const UDPConnection* connection(const NetAddress& addr) const;
  UDPConnection* connection(const NetAddress& addr);

protected:
  void finalize(NetMessage& msg);
  bool handle(NetMessage& msg, const NetAddress& addr);
  std::vector<UDPConnection> mConnections;
  std::vector<NetMessage::Def> mMessageDefs;
  std::unordered_map<std::string, message_handle_t> mHandles;
  UDPSocket mSock;
};

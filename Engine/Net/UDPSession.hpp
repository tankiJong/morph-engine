#pragma once
#include "Engine/Core/common.hpp"
#include <functional>
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/UDPSocket.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Net/UDPConnection.hpp"
#include <queue>

class NetMessage;
class NetPacket;

class UDPSession {
  friend class UDPConnection;
public:
  static constexpr uint8_t INVALID_CONNECTION_ID = 0xffui8;
  static constexpr double DEFAULT_SEND_FREQ = 20.0;
  struct MessageHandle {
    
  };

  struct Sender {
    UDPConnection* connection;
  };

  using message_handle_t = std::function<bool(NetMessage, Sender&)>;

  UDPSession();

  template<typename Func>
  MessageHandle on(const char* name, Func&& func) {
    mMessageDefs.push_back({ 0, name });

    std::sort(mMessageDefs.begin(), mMessageDefs.end(),
              [](NetMessage::Def& a, NetMessage::Def& b) {
      return a.name < b.name;
    });

    for(uint i = 0; i < mMessageDefs.size(); i++) {
      EXPECTS(i < 0xff);
      mMessageDefs[i].index = (uint8_t)i;
    }

    mHandles[name] = func;
    return MessageHandle();
  }

  bool bind(uint16_t port);

  bool connect(uint8_t index, const NetAddress& addr);

  bool send(uint8_t index, NetMessage& msg);

  bool in();

  bool out();

  bool send(uint8_t index, const NetPacket& packet);

  uint8_t selfIndex() const { return INVALID_CONNECTION_ID; }

  const UDPConnection* connection(uint8_t index) const;
  UDPConnection* connection(uint8_t index);

  const UDPConnection* connection(const NetAddress& addr) const;
  UDPConnection* connection(const NetAddress& addr);

  bool verify(const NetPacket& packet);

  void simLatency(uint minMs, uint maxMs = 0u);
  void simLoss(float percentage);

  double tickSecond() const { return mTickSecond; };
  double tickSecond(double tickSec);
  double tickFrequency(float freq);
  double connectionTickFrequency(uint8_t index, float freq);
  void heartbeatFrequency(float freq);
protected:
  struct NetPacketComp {
    bool operator()(const NetPacket* lhs, const NetPacket* rhs);
  };
  void finalize(NetMessage& msg);
  bool handle(NetMessage& msg, Sender& sender);
  void processPackets();

  NetPacket* allocPacket();
  void freePacket(NetPacket*& packet);

  std::vector<UDPConnection> mConnections;
  std::vector<NetMessage::Def> mMessageDefs;
  std::unordered_map<std::string, message_handle_t> mHandles;
  std::priority_queue<NetPacket*, std::vector<NetPacket*>, NetPacketComp> mPendingPackets;
  UDPSocket mSock;

  uint mMinSimLatencyMs = 0u, mMaxSimLatencyMs = 0u;
  float mSimLossChance = 0.f;
  double mTickSecond = 1.0 / DEFAULT_SEND_FREQ;
};


#pragma once
#include "Engine/Core/common.hpp"
#include <functional>
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/UDPSocket.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Net/UDPConnection.hpp"
#include <queue>
#include "Engine/Net/NetPacket.hpp"

class NetMessage;
class NetPacket;

enum eNetCoreMessage: uint8_t {
  NETMSG_PING = 0,    // unreliable, connectionless
  NETMSG_PONG, 		// unreliable, connectionless
  NETMSG_HEARTBEAT,	// unreliable

  NETMSG_CORE_COUNT,
};

struct UDPSender {
  UDPConnection* connection;
  NetAddress address;
  UDPSession* session;
};

class UDPSession {
  friend class UDPConnection;
public:
  static constexpr uint8_t INVALID_CONNECTION_ID = 0xffui8;
  static constexpr double DEFAULT_SEND_FREQ = 20;

  struct MessageHandle {
    
    uint16_t mOldestSentRelialbeId = UINT16_MAX;
  };

  using Sender = UDPSender;

  using message_handle_t = std::function<bool(NetMessage, Sender&)>;

  UDPSession();

  template<typename Func>
  MessageHandle on(const char* name, Func&& func, eMessageOption op = eMessageOption(0), uint8_t messageChannel = 0) {
    return on(NetMessage::Def::INVALID_MESSAGE_INDEX, name, func, op, messageChannel);
  }

  MessageHandle on(uint8_t index, const char* name, const message_handle_t& func, eMessageOption option = eMessageOption(0), uint8_t messageChannel = 0);


  bool bind(uint16_t port);

  bool connect(uint8_t index, const NetAddress& addr);

  bool send(uint8_t index, NetMessage& msg, bool needFlush = false);

  bool in();

  bool out();

  bool send(uint8_t index, const NetPacket& packet);

  uint8_t selfIndex() const { return mSelfIndex; }

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

  void renderUI() const;
protected:
  struct NetPacketComp {
    bool operator()(const NetPacket* lhs, const NetPacket* rhs);
  };
  void finalize(NetMessage& msg);
  bool handle(NetMessage& msg, Sender& sender);
  void processPackets();

  NetPacket* allocPacket();
  void freePacket(NetPacket*& packet);

  void registerCoreMessage();
  void finalizeMessageDefinition();

  std::vector<UDPConnection> mConnections;
  std::vector<NetMessage::Def> mIndexedMessageDefs;
  std::vector<NetMessage::Def> mUnIndexedMessageDefs;
  std::array<NetMessage::Def, 0xff> mMessageDefs;
  std::unordered_map<std::string, message_handle_t> mHandles;
  std::priority_queue<NetPacket*, std::vector<NetPacket*>, NetPacketComp> mPendingPackets;
  UDPSocket mSock;
  uint8_t mSelfIndex = INVALID_CONNECTION_ID;
  uint mMinSimLatencyMs = 0u, mMaxSimLatencyMs = 0u;
  float mSimLossChance = 0.f;
  double mTickSecond = 1.0 / DEFAULT_SEND_FREQ;

  uint16_t mNextSendeAck = 0u;

  uint16_t mLastReceivedAck = NetPacket::INVALID_PACKET_ACK;
  uint16_t mPreviousReceivedAckAckBitField = 0u;
};


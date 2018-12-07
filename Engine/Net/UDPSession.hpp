#pragma once
#include "Engine/Core/common.hpp"
#include <functional>
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/UDPSocket.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Net/UDPConnection.hpp"
#include <queue>
#include "Engine/Net/NetPacket.hpp"
#include <optional>
#include "Engine/Net/NetObject.hpp"

class NetMessage;
class NetPacket;

enum eNetCoreMessage: uint8_t {
  NETMSG_PING = 0,    // unreliable, connectionless
  NETMSG_PONG, 		// unreliable, connectionless
  NETMSG_HEARTBEAT,	// unreliable

  NETMSG_JOIN_REQUEST,
  NETMSG_JOIN_DENY,
  NETMSG_JOIN_ACCEPT,
  NETMSG_NEW_CONNECTION,
  NETMSG_JOIN_FINISHED,
  NETMSG_UPDATE_CONN_STATE,

  NETMSG_OBJECT_CREATE,
  NETMSG_OBJECT_DESTROY,
  NETMSG_OBJECT_UPDATE,

  NETMSG_CORE_COUNT,
};

struct UDPSender {
  UDPConnection* connection;
  NetAddress address;
  UDPSession* session;
};

enum eSessionState {
  SESSION_DISCONNECTED = 0,  // Session can be modified     
  SESSION_BOUND,             // Bound to a socket - can send and receive connectionless messages.  No connections exist
  SESSION_CONNECTING,        // Attempting to connecting - waiting for response from the host
  SESSION_JOINING,           // Has established a connection, waiting final setup information/join completion
  SESSION_READY,             // We are fully in the session
};

enum eSessionError {
  SESSION_OK,
  SESSION_ERROR_USER,                 // user disconnected

  SESSION_ERROR_INTERNAL,
  SESSION_ERROR_JOIN_TIMEOUT,
  SESSION_ERROR_JOIN_DENIED,          // generic deny error (release)
  SESSION_ERROR_JOIN_DENIED_NOT_HOST, // debug - tried to join someone who isn't hosting
  SESSION_ERROR_JOIN_DENIED_CLOSED,   // debug - not in a listen state
  SESSION_ERROR_JOIN_DENIED_FULL,     // debug - session was full 
};

class UDPSession {
  friend class UDPConnection;
  friend class NetObjectManager;
public:
  static constexpr uint8_t INVALID_CONNECTION_ID = 0xffui8;
  static constexpr double DEFAULT_SEND_FREQ = 60;
  static constexpr uint HOST_CONNECTION_INDEX = 0;
  static constexpr double JOIN_TIMEOUT_SEC = 10;
  static constexpr double CONNECTION_TIMEOUT_SEC = 5;
  static constexpr float MAX_NET_TIME_DILATION = .5f;
  struct MessageHandle {
    
    uint16_t mOldestSentRelialbeId = UINT16_MAX;
  };

  using Sender = UDPSender;

  using message_handle_t = std::function<bool(NetMessage, Sender&)>;

  UDPSession();
  ~UDPSession();
  template<typename Func>
  MessageHandle on(const char* name, Func&& func, eMessageOption op = eMessageOption(0), uint8_t messageChannel = 0) {
    return on(NetMessage::Def::INVALID_MESSAGE_INDEX, name, func, op, messageChannel);
  }

  MessageHandle on(uint8_t index, const char* name, const message_handle_t& func, eMessageOption option = eMessageOption(0), uint8_t messageChannel = 0);

  void host(const char* id, uint16_t port);
  void join(const char* id, const NetAddress& host);

  void disconnect();

  bool send(uint8_t index, NetMessage& msg, bool needFlush = false);
  bool send(uint8_t index, const NetPacket& packet);
  bool send(NetAddress& addr, NetMessage& msg);
  bool sendOthers(NetMessage& msg);
  bool in();

  bool out();


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

  eSessionError err();

  bool hasErr() const { return mLastError != SESSION_OK; }

  UDPConnection* hostConnection();
  UDPConnection* selfConnection();

  bool isRunning() const {
    return mSessionState != SESSION_DISCONNECTED;
  }

  bool isHosting() const;
  bool isHosted() const;
  uint64_t sessionTimeMs() const;

  NetObjectManager& netObjectManager() { return mNetObjectManager; }

  template<typename Func>
  void onJoin(Func&& func) { mJoinCb.emplace_back(func); }

  template<typename Func>
  void onLeave(Func&& func) { mLeaveCb.emplace_back(func); }

  void syncObjects();
  // void updateObject(net_object_local_object_t* obj);
protected:
  bool connect(uint8_t index, const NetAddress& addr);
  void err(eSessionError errorCode);

  void sessionState(eSessionState state);
  eSessionState sessionState() const { return mSessionState; }
  bool bind(uint16_t port);
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

  // void updateNetObjects();

  NetObject::View createView(NetObject& netObject);
  NetObject::View cloneView(const NetObject::View& view);
  void destoryView(NetObject::View& view);
  void registerToConnections(const NetObject::View& view);
  void unregisterFromConnections(const NetObject& obj);

  bool onJoinAccepted(NetMessage msg, UDPSession::Sender& sender);
  std::optional<uint8_t> aquireNextAvailableConnection();
  std::array<UDPConnection, INVALID_CONNECTION_ID + 1> mConnections;
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

  eSessionState mSessionState = SESSION_DISCONNECTED;
  eSessionError mLastError = SESSION_OK;

  double mJoinTimeout = 0;

  uint64_t mDesiredClientMilliSec;
  uint64_t mCurrentClientMilliSec;

  NetObjectManager mNetObjectManager; 

  std::vector<std::function<void(UDPSession&, UDPConnection&)>> mJoinCb;
  std::vector<std::function<void(UDPSession&, UDPConnection&)>> mLeaveCb;
};


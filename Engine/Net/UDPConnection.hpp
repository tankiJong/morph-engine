#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include <vector>
#include "Engine/Net/NetPacket.hpp"
#include <bitset>
#include "Engine/Net/UDPSession.hpp"

class NetMessage;
class UDPSession;
class UDPSender;
class NetMessageChannel {
public:
  uint16_t nextSendSequenceId = 0;
  uint16_t nextExpectReceiveSequenceId = 0;
  std::vector<NetMessage>outOfOrderMessages;
};


class UDPConnection {
  friend class NetPacket;
public:
  static constexpr uint16_t RELIALBE_WINDOW_SIZE = 64;
  static constexpr uint8_t MAX_MESSAGE_CHANNEL_COUNT = 8;
  struct PacketTracker {
    static constexpr uint16_t MAX_RELIABLES_PER_PACKET = 32;
    double sendSec;
    uint16_t ack = NetPacket::INVALID_PACKET_ACK;
    bool isUsing = false;
    void reset() {
      ack = NetPacket::INVALID_PACKET_ACK;
      isUsing = false;
      mSentReliableCount = 0;
    }

    span<const uint16_t> reliables() const;
    void bind(const NetPacket& packet);

    bool occupied() const { return isUsing; }
    bool occupied(uint _ack) const { return ack == _ack && isUsing; }

  protected:
    std::array<uint16_t, MAX_RELIABLES_PER_PACKET> mSentReliable;
    uint16_t mSentReliableCount = 0;
    void registerReliable(const NetMessage* msg);
  };

  static constexpr double DEFAULT_HEARTBEAT_RATE = 5.0;
  static constexpr double DEFAULT_RELIABLE_RESEND_SEC = .1f;
  bool valid() const { return mOwner != nullptr; }

  bool send(NetMessage& msg);
  bool flush(bool force = false);
  const NetAddress& addr() const { return mAddress; }


  bool set(UDPSession& session, uint8_t index, const NetAddress& addr);

  uint16_t nextAck() const { return mNextAckToUse; }

  float rtt() const { return float(mRtt); }
  float rtt(double rtt);
  float lossRate() const { return mLostRate; }

  uint16_t previousReceivedAckBitField() const;

  double lastReceiveSecond() const { return mLastReceivedSec; }
  void lastReceiveSecond(double sec) { mLastReceivedSec = sec; }

  double lastSendSecond() const { return mLastSendSec; }
  
  uint16_t lastSendAck() const { return mLastSendAck; }
  uint16_t largestReceivedAck() const { return mLargestReceivedAck;  }

  bool process(NetMessage& msg, UDPSender& sender);
  uint8_t indexOfSession() const { return mIndexOfSession; };

  void heartbeatFrequency(double freq);
  double tickFrequency(double freq);
  UDPSession* owner() const { return mOwner; };

  bool onReceive(const NetPacket::header_t& header);

  bool isReliableReceived(uint16_t reliableId) const;
  size_t pendingReliableCount() const;

  const NetMessageChannel& messageChannel(uint index) const { return mMessageChannels[index]; }
protected:
  static constexpr uint PACKET_TRACKER_CACHE_SIZE = 64;

  uint16_t increaseAck();;
  PacketTracker& track(NetPacket& packet);
  bool shouldSendPacket() const;
  bool tryAppendHeartbeat();
  bool confirmReceived(uint16_t ack);
  PacketTracker& packetTracker(uint ack);
  bool canSendNewReliable() const;

  NetAddress mAddress;
  UDPSession* mOwner = nullptr;
  uint8_t mIndexOfSession = 0xffui8;
  Interval mHeartBeatTimer{ 1.f / DEFAULT_HEARTBEAT_RATE };

  double mLastSendSec = 0;
  double mLastReceivedSec = 0;
  double mTickSec = 0;

  uint16_t mNextReliableId = 0;
  uint16_t mNextAckToUse = NetPacket::INVALID_PACKET_ACK;
  uint16_t mLastSendAck = 0;
  uint16_t mLargestReceivedAck = NetPacket::INVALID_PACKET_ACK;
  uint16_t mReceivedBitField = 0;
  double mRtt = 0;

  float mLostRate = 0;
  std::array<PacketTracker, PACKET_TRACKER_CACHE_SIZE> mTrackers;
  std::array<bool, PACKET_TRACKER_CACHE_SIZE> mLostPacketMarker{false};
  
  std::vector<NetMessage> mOutboundUnreliables;

  std::vector<NetMessage> mUnsentReliable;
  std::vector<NetMessage> mSentReliable;

  std::bitset<0x10000> mReceivedReliableMessage;
  uint16_t mOldestUnconfirmedRelialbeId = UINT16_MAX;

  uint16_t mHighestReceivedReliableId = UINT16_MAX;

  std::array<NetMessageChannel, MAX_MESSAGE_CHANNEL_COUNT> mMessageChannels;
};


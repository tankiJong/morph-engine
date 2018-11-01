#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include <vector>
#include "Engine/Net/NetPacket.hpp"

class NetMessage;
class UDPSession;



class UDPConnection {
  friend class NetPacket;
public:

  struct PacketTracker {
    double sendSec;
    uint16_t ack = NetPacket::INVALID_PACKET_ACK;
    bool isUsing = false;
    void reset() {
      ack = NetPacket::INVALID_PACKET_ACK;
      isUsing = false;
    }

    void bind(const NetPacket& packet);

    bool occupied() const { return isUsing; }
    bool occupied(uint _ack) const { return ack == _ack && isUsing; }
  };
  static constexpr double DEFAULT_HEARTBEAT_RATE = 5.0;
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
  uint16_t largestReceivedAck() const { return mLargestReceivedAck;  };
  uint8_t indexOfSession() const { return mIndexOfSession; };

  void heartbeatFrequency(double freq);
  double tickFrequency(double freq);
  UDPSession* owner() const { return mOwner; };

  bool onReceive(const NetPacket::header_t& header);

protected:
  static constexpr uint PACKET_TRACKER_CACHE_SIZE = 64;

  uint16_t increaseAck();;
  PacketTracker& track(NetPacket& packet);
  bool shouldSendPacket() const;
  bool tryAppendHeartbeat();
  bool confirmReceived(uint16_t ack);
  PacketTracker& packetTracker(uint ack);
  std::vector<NetMessage> mOutboundUnreliables;
  NetAddress mAddress;
  UDPSession* mOwner = nullptr;
  uint8_t mIndexOfSession = 0xffui8;
  Interval mHeartBeatTimer{ 1.f / DEFAULT_HEARTBEAT_RATE };
  double mLastSendSec = 0;
  double mLastReceivedSec = 0;
  double mTickSec = 0;
  uint16_t mNextAckToUse = NetPacket::INVALID_PACKET_ACK;
  uint16_t mLastSendAck = 0;
  uint16_t mLargestReceivedAck = NetPacket::INVALID_PACKET_ACK;
  uint16_t mReceivedBitField = 0;
  double mRtt = 0;
  std::array<PacketTracker, PACKET_TRACKER_CACHE_SIZE> mTrackers;
  std::array<bool, PACKET_TRACKER_CACHE_SIZE> mLostPacketMarker{false};
  float mLostRate = 0;
};


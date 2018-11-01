#include "UDPConnection.hpp"
#include "Engine/Net/NetPacket.hpp"
#include "Engine/Net/UDPSession.hpp"
#include "Engine/Debug/Log.hpp"

void UDPConnection::PacketTracker::bind(const NetPacket& packet) {
  sendSec = GetCurrentTimeSeconds();
  ack = packet.ack();
  isUsing = true;
}

bool UDPConnection::send(NetMessage& msg) {
  mOwner->finalize(msg);
  mOutboundUnreliables.push_back(msg);

  return true;
}

bool UDPConnection::flush(bool force) {
  tryAppendHeartbeat();
  if(!force) {
    if (!shouldSendPacket()) return false;
  }

  NetPacket packet;

  // TODO: change how I construct the packet, this logic is buggy
  packet.begin(*this);

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

  track(packet);

  mOwner->send(mIndexOfSession, packet);

  increaseAck();
  mLastSendSec = GetCurrentTimeSeconds();
  
  mOutboundUnreliables.clear();

  return true;
}

bool UDPConnection::set(UDPSession& session, uint8_t index, const NetAddress& addr) {
  mOwner = &session;
  mIndexOfSession = index;
  mAddress = addr;
  mHeartBeatTimer.flush();
  return true;
}

float UDPConnection::rtt(double rtt) {
  mRtt = rtt;
  return float(mRtt);
}

uint16_t UDPConnection::previousReceivedAckBitField() const {
  return mReceivedBitField;
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

bool UDPConnection::onReceive(const NetPacket::header_t& header) {
  mLastReceivedSec = GetCurrentTimeSeconds();

  {
    // process bit field, confirm packets
    uint16_t bf = header.previousReceivedAckBitField;
    uint16_t ack = header.lastReceivedAck;
 
    uint received = 1;
    while(bf > 0 || received) {
      if(received) {
        confirmReceived(ack);
      }
      ack--;
      received = bf & 0x1;
      bf >>= 1;
    }
  }


  {
    uint16_t ack = header.ack;
    // update the data which will carry out in the next packet
    if (ack - mLargestReceivedAck <= 0x8000) {
      uint shift = ack - mLargestReceivedAck;
      mReceivedBitField <<= shift;
      uint16_t mask = 0x1 << (shift - 1u);
      mReceivedBitField = mReceivedBitField | mask;
      mLargestReceivedAck = ack;
    } else {
      uint shift = mLargestReceivedAck - ack - 1;
      uint16_t mask = 0x1 << shift;
      mReceivedBitField = mReceivedBitField | mask;
    }
  }

  return true;
}

uint16_t UDPConnection::increaseAck() {
  mNextAckToUse = mNextAckToUse + 1u;

  if(mNextAckToUse % PACKET_TRACKER_CACHE_SIZE == 0) {
    uint lost = 0;

    for(bool marker: mLostPacketMarker) {
      if(marker) {
        lost++;
      }
    }

    mLostRate = float(lost) / float(PACKET_TRACKER_CACHE_SIZE);
    mLostPacketMarker.fill(false);
  }

  return mNextAckToUse;
}

UDPConnection::PacketTracker& UDPConnection::track(NetPacket& packet) {

  mLastSendAck = packet.ack();

  uint index = mLastSendAck % PACKET_TRACKER_CACHE_SIZE;
 
  PacketTracker& tracker = mTrackers[index];

  if(tracker.occupied()) {
    // handle lost
    mLostPacketMarker[index] = true;
  }

  tracker.bind(packet);

  return tracker;
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

bool UDPConnection::confirmReceived(uint16_t ack) {
  // if (ack == NetPacket::INVALID_PACKET_ACK) {
  //   Log::logf("try to confirm receive invalid ack");
  //   return false;
  // }

  PacketTracker& pt = packetTracker(ack);
  if (!pt.occupied(ack)) return false;


  // compute rtt and update tracker
  if(ack - mLargestReceivedAck <= 0x8000) {
    double packetRtt = GetCurrentTimeSeconds() - pt.sendSec;
    // Log::logf("send: %lf, rtt: %lf", tracker.sendSec, packetRtt);
    rtt(packetRtt);
    // Log::logf("[%u]computed rtt: %lf, ack: %u, largestReceivedAck: %u", mIndexOfSession, rtt(), ack, mLargestReceivedAck);
  }
  pt.reset();

  return true;
}

UDPConnection::PacketTracker& UDPConnection::packetTracker(uint ack) {
  return mTrackers[ack % PACKET_TRACKER_CACHE_SIZE];
}

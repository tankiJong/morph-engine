#include "UDPSession.hpp"
#include "UDPConnection.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Debug/Console/Command.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Net/NetPacket.hpp"
#include "Game/Gameplay/Map.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Renderer/Geometry/Mesher.hpp"
#include "Engine/Application/Window.hpp"

bool onHeartBeat(NetMessage, UDPSession::Sender&) {
  return true;
}

UDPSession::UDPSession() {
  
  // bind core message
  // * heartbeat
  
  registerCoreMessage();
}

UDPSession::MessageHandle UDPSession::on(uint8_t index, const char* name, const message_handle_t& func, eMessageOption option, uint8_t messageChannel) {
  if(index == NetMessage::Def::INVALID_MESSAGE_INDEX) {
    mUnIndexedMessageDefs.push_back({ NetMessage::Def::INVALID_MESSAGE_INDEX, messageChannel, name, option });
  } else {
    mIndexedMessageDefs.push_back({ index, messageChannel, name, option });
  }

  finalizeMessageDefinition();

  mHandles[name] = func;

  return MessageHandle();
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

  if(mConnections[index].valid()) {
    Log::errorf("the request connection is already binded.");
    return false;
  }

  if(addr == mSock.address()) {
    mSelfIndex = index;
  }
  return mConnections[index].set(*this, index, addr);
}

bool UDPSession::send(uint8_t index, NetMessage& msg, bool needFlush) {
  EXPECTS(index < mConnections.size());

  if(!mConnections[index].valid()) {
    Log::tagf("net", "Fail to send message to an invalid connection [%u]", index);
    return false;
  }

  bool result = mConnections[index].send(msg);

  if(needFlush) {
    mConnections[index].flush(true);
  }

  return result;
}

bool UDPSession::in() {
  byte_t buf[NET_PACKET_MTU];
  NetAddress addr;

  while(true) {
    size_t size = mSock.receive(addr, buf, NET_PACKET_MTU);

    if(size == 0) {
      break;
    }

    if(addr.port() == 10084u) {
      int i = 0;
      i++;
    }

    NetPacket* packet = allocPacket();
    packet->fill(buf, size);

    if(!verify(*packet)) {
      Log::tagf("net", "Received invalid traffic from %s", addr.toString());
      return false;
    }

    // ----- sim ------
    if (checkRandomChance(mSimLossChance)) {
      freePacket(packet);
      continue;
    }

    float latency = getRandomf(
      (float)mMinSimLatencyMs, 
      (float)std::max(mMinSimLatencyMs, mMaxSimLatencyMs));
    ENSURES(latency >= mMinSimLatencyMs && latency <= mMaxSimLatencyMs)
    double currentTime = GetCurrentTimeSeconds();
    packet->receivedTime(currentTime + latency * 0.001f);

    mPendingPackets.push(packet);
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

  for(uint8_t i = 0; i < header.reliableCount; i++) {
    NetMessage msg;
    bool valid = p.read(mMessageDefs, msg);
    if (!valid) {
      Log::logf("received packet contains invalid reliable message");
      return false;
    }
  }

  for (uint8_t i = 0; i < header.unreliableCount; i++) {
    NetMessage msg;
    bool valid = p.read(mMessageDefs, msg);
    if (!valid) {
      Log::logf("received invalid packet contains invalid unreliable message");
      return false;
    }
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

void UDPSession::renderUI() const {
  Renderer& renderer = *Renderer::Get();

  auto font = Font::Default();

  Mesher ms;
  constexpr float LINE_PADDING = 10.f;
  aabb2 bound = Window::Get()->bounds();

  vec3 cursorStart = { 10.f, bound.height() - LINE_PADDING - font->lineHeight(30.f), 0 };

  ms.begin(DRAW_TRIANGES);


  ms.text("SESSION INFO", 20.f, font.get(), cursorStart);
  cursorStart -= { 0, LINE_PADDING + font->lineHeight(20.f), 0 };

  ms.text(
    Stringf("sim lag: %.0fms ~ %.0fms     sim loss: %.2f%%", float(mMinSimLatencyMs), float(mMaxSimLatencyMs), float(mSimLossChance) * 100.f),
    16.f, font.get(), cursorStart);
  cursorStart -= { 0, LINE_PADDING + font->lineHeight(16.f), 0 };

  ms.color(Rgba(100, 100, 255));
  ms.text(
    Stringf("Self Address: %s", mSock.address().toString()),
    18.f, font.get(), cursorStart);
  cursorStart -= { 0, LINE_PADDING + font->lineHeight(18.f), 0 };


  ms.color(Rgba(200, 200, 200));
  const char* formatStr        = "%-8s%-25s%-10s%-10s%-10s%-10s%-10s%-10s%-30s%-10s%-10s";
  const char* contentFormatStr = "%-8u%-25s%-10s%-10.3f%-10.3lf%-10.3lf%-10u%-10u%-30s%-10u%-10u";
  ms.text("==== Connections ====", 18.f, font.get(), cursorStart);
  cursorStart -= { 0, LINE_PADDING + font->lineHeight(18.f), 0 };

  ms.color(Rgba::white);
  ms.text(Stringf(formatStr, "idx", "address", "rtt", "loss", "lrcv(s)", "lsnt(s)", "sntack", "rcvack", "rcvbits", "reliables", "stored_inorder0"), 16.f, font.get(), cursorStart);
  cursorStart -= { 0, LINE_PADDING + font->lineHeight(16.f), 0 };

  ms.color(Rgba(200, 180, 180));
  for(const UDPConnection& connection: mConnections) {
    if (!connection.valid()) continue;
    ms.color(connection.indexOfSession() == selfIndex() ? Rgba(100, 180, 180) : Rgba(200, 180, 180));

    char buff[20];
    memset(buff, '0', 20);

    {
      char buf[17];
      _itoa_s(connection.previousReceivedAckBitField(), buf, 2);
      size_t len = strlen(buf) + 1;
      memcpy(buff + 17 - len, buf, len);
    }

    ms.text(
      Stringf(contentFormatStr,
              connection.indexOfSession(), connection.addr().toString(),
              beautifySeconds(connection.rtt()).c_str(), connection.lossRate(), 
              GetCurrentTimeSeconds() - connection.lastReceiveSecond(),
              GetCurrentTimeSeconds() - connection.lastSendSecond(), 
              connection.lastSendAck(), connection.largestReceivedAck(),
              buff, connection.pendingReliableCount(), connection.messageChannel(0).outOfOrderMessages.size()), 16.f, font.get(), cursorStart);
    cursorStart -= { 0, LINE_PADDING + font->lineHeight(16.f), 0 };
  }
  
  ms.end();
  Mesh* mesh = ms.createMesh<vertex_pcu_t>();
  renderer.setMaterial(Resource<Material>::get("material/ui/font").get());
  renderer.setTexture(font->texture());
  renderer.drawMesh(*mesh);

  delete mesh;
}

bool UDPSession::NetPacketComp::operator()(const NetPacket* lhs, const NetPacket* rhs) {
  return (*lhs) > (*rhs);
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
  double currentTime = GetCurrentTimeSeconds();
  while(!mPendingPackets.empty()) {
    NetPacket* packet = mPendingPackets.top();
    if (packet->receivedTime() > currentTime) break;
    mPendingPackets.pop();

    NetPacket::header_t header{};
    packet->read(header);

    UDPConnection* conn = connection(header.connectionIndex);
    conn->onReceive(header);

    Sender sender{ conn, packet->senderAddr(), this };

    for(uint i = 0; i < header.reliableCount; i++) {
      NetMessage msg;
      packet->read(mMessageDefs, msg);
      finalize(msg);

      EXPECTS(msg.name().length() > 0);


      if (conn != nullptr) {
        conn->process(msg, sender);
        //
        // bool shouldHandle = conn->process(msg);
        //
        // if (shouldHandle) {
        //   handle(msg, sender);
        // }

      } else {

        if (msg.connectionless()) {
          handle(msg, sender);
        } else {
          Log::log("receive message which require a connection, throw away");
        }

      }
    }

    for (uint i = 0; i < header.unreliableCount; i++) {
      NetMessage msg;
      packet->read(mMessageDefs, msg);
      finalize(msg);

      EXPECTS(msg.name().length() > 0);

      if (conn != nullptr) {

        handle(msg, sender);

      } else {

        if(msg.connectionless()) {
          handle(msg, sender);
        } else {
          Log::log("receive message which require a connection, throw away");
        }

      }

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

void UDPSession::registerCoreMessage() {
  on(uint8_t(NETMSG_PING), "ping", [](NetMessage msg, UDPSession::Sender& sender) {

    char buf[1000];

    msg.read(buf, 1000);

    Log::logf("%s", buf);

    NetMessage m("pong");

    sender.connection->send(m);
    return true;
  }, NETMESSAGE_OPTION_CONNECTIONLESS);

  on(uint8_t(NETMSG_PONG), "pong", [](NetMessage msg, UDPSession::Sender& sender) {
    Log::logf("Pong");
    return true;
  }, NETMESSAGE_OPTION_CONNECTIONLESS);

  on(uint8_t(NETMSG_HEARTBEAT), "heartbeat", onHeartBeat);
}

void UDPSession::finalizeMessageDefinition() {

  mMessageDefs.fill(NetMessage::Def());

  for(NetMessage::Def& def: mIndexedMessageDefs) {
    EXPECTS(mMessageDefs[def.index].index == NetMessage::Def::INVALID_MESSAGE_INDEX);
    mMessageDefs[def.index] = def;
  }

  std::sort(mIndexedMessageDefs.begin(), mIndexedMessageDefs.end(),
            [](NetMessage::Def& a, NetMessage::Def& b) {
    return a.name < b.name;
  });

  auto unindexedIter = mUnIndexedMessageDefs.begin();
  
  for (uint8_t i = 0; i < mMessageDefs.size(); i++) {
    if (unindexedIter == mUnIndexedMessageDefs.end()) break;
    if(mMessageDefs[i].index == NetMessage::Def::INVALID_MESSAGE_INDEX) {
      mMessageDefs[i] = *unindexedIter;
      ++unindexedIter;
    }
  }
}

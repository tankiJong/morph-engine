#include "UDPSession.hpp"
#include "UDPConnection.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Debug/Console/Command.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Net/NetPacket.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Graphics/Model/Mesher.hpp"
#include "Engine/Graphics/Model/Mesh.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Renderer/ImmediateRenderer.hpp"

#include <optional>

UDPSession::UDPSession() {
  
  registerCoreMessage();
  mNetObjectManager.init(*this);
}

UDPSession::~UDPSession() {
  disconnect();
  for(UDPConnection& connection: mConnections) {
    connection.flush(true);
  }
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

void UDPSession::host(const char* /*id*/, uint16_t port) {
  bind(port);
  bool connected = connect(0, mSock.address());
  if(connected) {
    connection(0)->connectionState(CONNECTION_READY);
    sessionState(SESSION_READY);
    for(const auto& cb: mJoinCb) {
      cb(*this, *connection(0));
    }
  } else {
    sessionState(SESSION_DISCONNECTED);
    mLastError = SESSION_ERROR_INTERNAL;
  }

}

void UDPSession::join(const char* /*id*/, const NetAddress& host) {
  bind(host.port());

  connect(HOST_CONNECTION_INDEX, host);
  connect(INVALID_CONNECTION_ID, mSock.address());
  selfConnection()->connectionState(CONNECTION_CONNECTING);
  sessionState(SESSION_CONNECTING);
}

void UDPSession::disconnect() {
  if(sessionState() != SESSION_DISCONNECTED) {
    NetMessage message(NETMSG_UPDATE_CONN_STATE);
    uint8_t index = selfIndex();
    uint8_t state = uint8_t(CONNECTION_DISCONNECTED);

    message << index << state;

    sendOthers(message);

    for(UDPConnection& connection: mConnections) {
      if (!connection.valid()) continue;

      connection.disconnect();
    }

    sessionState(SESSION_DISCONNECTED);
  }
}

bool UDPSession::isHosting() const {
  return
    mSessionState == SESSION_READY && mSelfIndex == HOST_CONNECTION_INDEX;
}

bool UDPSession::isHosted() const {
  return
    mSessionState == SESSION_READY && 
    mSelfIndex != HOST_CONNECTION_INDEX;
}

uint64_t UDPSession::sessionTimeMs() const {
  if(isHosting()) {
    return (uint64_t)GetMainClock().total.second * 1000;
  }

  return mCurrentClientMilliSec;
}

void UDPSession::sessionState(eSessionState state) {
  mSessionState = state;
}

bool UDPSession::bind(uint16_t port) {
  if(!mSock.closed()) {
    mSock.close();
  }

  bool re = mSock.bind(NetAddress::local(port), 100);
  mSock.unsetOption(SOCKET_OPTION_BLOCKING);

  sessionState(SESSION_BOUND);
  return re;
}

// void UDPSession::updateObject(net_object_local_object_t* obj) {
//   NetObject* netObject = mNetObjectManager.find(obj);
//   if(netObject != nullptr) {
//     selfConnection()->netObjectViewCollection().update(netObject, mNetObjectManager);
//   } else {
//     Log::log("try to update an object which is not registered in net object system");
//   }
// }

void UDPSession::syncObjects() {
  if(isHosted()) {
    mNetObjectManager.applySnapshots();
  } else {
    mNetObjectManager.updateSnapshots();
  }

}

bool UDPSession::connect(uint8_t index, const NetAddress& addr) {
  EXPECTS(index < mConnections.size());

  if(mConnections[index].valid()) {
    Log::errorf("the request connection is already binded.");
    return false;
  }

  if(addr == mSock.address()) {
    mSelfIndex = index;
  }
  bool connected = mConnections[index].set(*this, index, addr);

  if (connected) mConnections[index].connectionState(CONNECTION_CONNECTED);
}

void UDPSession::err(eSessionError errorCode) {
  mLastError = errorCode;
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


  if(mSessionState == SESSION_CONNECTING) {
    mJoinTimeout += GetMainClock().frame.second;
    UDPConnection& connection = *selfConnection();

    if(mJoinTimeout <= JOIN_TIMEOUT_SEC) {
      if(connection.connected()) {
        sessionState(SESSION_JOINING);
        mJoinTimeout = 0;
      } else {
        NetMessage msg(NETMSG_JOIN_REQUEST);
        if(hostConnection()->valid()) {
          hostConnection()->send(msg);
        }
      }
    } else {
      // timeout
      mLastError = SESSION_ERROR_JOIN_TIMEOUT;
      connection.disconnect();
      disconnect();
      mJoinTimeout = 0;
    }

  }

  if(mSessionState == SESSION_JOINING) {
    UDPConnection& connection = *selfConnection();
    if(connection.connected()) {
      sessionState(SESSION_READY);
      for(const auto& cb: mJoinCb) {
        cb(*this, connection);
      }
    }
  }
  while(true) {
    size_t size = mSock.receive(addr, buf, NET_PACKET_MTU);

    if(size == 0) {
      break;
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
    packet->senderAddr(addr);
    mPendingPackets.push(packet);
  }

  processPackets();

  return true;
}

bool UDPSession::out() {
  if(!isHosting()) {
    uint64_t localDtMs = uint64_t(GetMainClock().frame.second * 1000);
    mDesiredClientMilliSec += localDtMs;

    uint64_t scaledLocalDtMs = localDtMs;

    if(mCurrentClientMilliSec + scaledLocalDtMs > mDesiredClientMilliSec) {
      uint64_t delta = mCurrentClientMilliSec + scaledLocalDtMs - mDesiredClientMilliSec;

      float scaleFactor = float(delta) / 200.f * MAX_NET_TIME_DILATION;
      scaleFactor = clamp(scaleFactor, 0.f, MAX_NET_TIME_DILATION);

      scaledLocalDtMs = uint64_t(float(scaledLocalDtMs) * ( 1.f - scaleFactor ));
    }
    if(mCurrentClientMilliSec + scaledLocalDtMs < mDesiredClientMilliSec) {
      uint64_t delta = mDesiredClientMilliSec - (mCurrentClientMilliSec + scaledLocalDtMs);

      float scaleFactor = float(delta) / 200.f * MAX_NET_TIME_DILATION;
      scaleFactor = clamp(scaleFactor, 0.f, MAX_NET_TIME_DILATION);

      scaledLocalDtMs = uint64_t(float(scaledLocalDtMs) * ( 1.f + scaleFactor ));
    }
    
    mCurrentClientMilliSec += scaledLocalDtMs;
  }

  for(UDPConnection& connection: mConnections) {
    if (!connection.valid()) continue;
    connection.flush();

    if(connection.indexOfSession() == selfIndex()) {
      continue;
    }
    if(GetCurrentTimeSeconds() - connection.lastReceiveSecond() > CONNECTION_TIMEOUT_SEC) {
      connection.disconnect();
    }
  }

  return true;
}

bool UDPSession::send(uint8_t index, const NetPacket& packet) {
  size_t size = mSock.send(
    mConnections[index].addr(), 
    packet.data(), packet.size());

  return size > 0;
}

bool UDPSession::send(NetAddress& addr, NetMessage& msg) {

  NetPacket packet;
  finalize(msg);
  packet.append(msg);
  packet.end();
  
  size_t size = mSock.send(addr, packet.data(), packet.size());

  return size > 0;
}

bool UDPSession::sendOthers(NetMessage& msg) {
  bool success = true;
  for(UDPConnection& connection: mConnections) {
    if(connection.valid() && &connection != selfConnection()) {
      success = success && connection.send(msg);
    }
  }

  return success;
}

const UDPConnection* UDPSession::connection(uint8_t index) const {
  if(index >= mConnections.size()) return nullptr;
  return &mConnections[index];
}

UDPConnection* UDPSession::connection(uint8_t index) {
  if (index >= mConnections.size()) return nullptr;
  return mConnections[index].valid() ? &mConnections[index] : nullptr;
}


const UDPConnection* UDPSession::connection(const NetAddress& addr) const {
  return const_cast<UDPSession*>(this)->connection(addr);
}

UDPConnection* UDPSession::connection(const NetAddress& addr) {
  for(UDPConnection& connection: mConnections) {
    if(connection.addr() == addr && connection.valid()) {
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
  //Renderer& renderer = *Renderer::Get();

  auto font = Font::Default();

  Mesher ms;
  constexpr float LINE_PADDING = 10.f;
  aabb2 bound = Window::Get()->bounds();

  vec3 cursorStart = { 10.f, bound.height() - LINE_PADDING - font->lineHeight(30.f), 0 };

  ms.begin(DRAW_TRIANGES);


  ms.text(Stringf("SESSION INFO - Time: %u", sessionTimeMs()), 20.f, font.get(), cursorStart);
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
  // renderer.setMaterial(Resource<Material>::get("material/ui/font").get());
  // renderer.setTexture(font->texture());
  // renderer.drawMesh(*mesh);

  delete mesh;
}

eSessionError UDPSession::err() {
  eSessionError error = mLastError;
  mLastError = SESSION_OK;
  return error;
}

UDPConnection* UDPSession::hostConnection() {
  return &mConnections[HOST_CONNECTION_INDEX];
}

UDPConnection* UDPSession::selfConnection() {
  return mSessionState == SESSION_DISCONNECTED  
       ? nullptr
       : &mConnections[mSelfIndex];
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

    if(conn != nullptr) {
      conn->onReceive(header);
    }

    Sender sender{ conn, packet->senderAddr(), this };

    for(uint i = 0; i < header.reliableCount; i++) {
      NetMessage msg;
      packet->read(mMessageDefs, msg);
      finalize(msg);

      EXPECTS(msg.name().length() > 0);

      if (conn != nullptr) {
        conn->process(msg, sender);
      } else {
        if(msg.connectionless()) {
          conn->process(msg, sender);
        } else {
          // Log::logf("receive message %u which require a connection, throw away", msg.index());
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
          // Log::logf("receive message %u which require a connection, throw away", msg.index());
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

  on(uint8_t(NETMSG_PONG), "pong", [](NetMessage msg, UDPSession::Sender&) {
    Log::logf("Pong");
    return true;
  }, NETMESSAGE_OPTION_CONNECTIONLESS);

  on(uint8_t(NETMSG_HEARTBEAT), "heartbeat", [](NetMessage msg, UDPSession::Sender& sender) {
    if(!sender.session->isHosting()) {
      uint64_t expectTime;
      msg >> expectTime;

      sender.session->mDesiredClientMilliSec = 
        expectTime + uint64_t(sender.connection->rtt() * .5f * 1000.f);
    }

    return true;
  });

  on(uint8_t(NETMSG_JOIN_REQUEST), "join_request", [](NetMessage msg, UDPSession::Sender& sender) {

    // ignore if already connected
    if(sender.session->connection(sender.address) != nullptr) {
      return true;
    }

    bool canJoin = true;
    eSessionError err;
    // can i host
    canJoin = sender.session->isHosting();
    if(!canJoin) {
      err = SESSION_ERROR_JOIN_DENIED_NOT_HOST;
      Log::warnf("will deny because session is not hosting");
    }
    // do i have more room
    std::optional<uint8_t> index = sender.session->aquireNextAvailableConnection();
    if(!index) {
      err = SESSION_ERROR_JOIN_DENIED_FULL;
      Log::warnf("will deny because session is full");
    }

    canJoin = canJoin && index;
    if(!canJoin) {
      Log::logf("deny join request from %s", sender.address.toString());

      NetMessage message(NETMSG_JOIN_DENY);

      uint8_t errCode = (uint8_t)err;
      msg << errCode;
      
      sender.session->send(sender.address, message);
      return true;
    }

    bool success = sender.session->connect(index.value(), sender.address);

    if(!success) {
      Log::warnf("failed to connect to %s, which JUST request to join", sender.address.toString());
    }

    for(const auto& cb: sender.session->mJoinCb) {
      cb(*sender.session, *sender.session->connection(index.value()));
    }
    auto views = sender.session->selfConnection()->netObjectViewCollection().views();
    NetObject::ViewCollection& colleciton = sender.session->connection(index.value())->netObjectViewCollection();

    for(auto& view: views) {
      colleciton.add(sender.session->cloneView(view));
    }

    Log::logf("accept join request from %s", sender.address.toString());
    NetMessage message(NETMSG_JOIN_ACCEPT);
    message << index.value();
    message << sender.session->sessionTimeMs();

    sender.session->connection(index.value())->send(message);

    return true;  
  }, NETMESSAGE_OPTION_CONNECTIONLESS);


  on(uint8_t(NETMSG_JOIN_ACCEPT), "join_accept", [this](NetMessage msg, Sender& sender) {
    Log::log("receive join accept message");

    bool success = sender.session->onJoinAccepted(msg, sender);
    
    EXPECTS(sender.session->selfConnection()->connectionState() == CONNECTION_CONNECTED);

    NetMessage message(NETMSG_UPDATE_CONN_STATE);
    uint8_t index = sender.session->selfIndex();
    uint8_t state = uint8_t(CONNECTION_CONNECTED);
    
    message << index << state;
    success = success && sender.session->sendOthers(message);

    return success;

  }, NETMSSAGE_OPTION_RELIALBE_IN_ORDER, 1);

  on(uint8_t(NETMSG_JOIN_DENY), "join_denied", [](NetMessage msg, UDPSession::Sender& sender) {

    if(sender.session->sessionState() != SESSION_CONNECTING) {
      // Log::logf("not in joining state, ignore deny msg. state: %u", sender.session->sessionState());
      return true;
    }

    uint8_t errCode;
    msg >> errCode;

    Log::errorf("the join request got denied, code: %u", errCode);
    sender.session->err(SESSION_ERROR_JOIN_DENIED);
    sender.session->disconnect();
    return true;
  });

  on(uint8_t(NETMSG_UPDATE_CONN_STATE), "update_conn_state", [](NetMessage msg, UDPSession::Sender& sender) {
    uint8_t connectionIndex, newState;
    
    msg >> connectionIndex >> newState;
    
    Log::logf("receive state sync msg from %u, new state: %u", connectionIndex, newState);
    
    UDPConnection* conn = sender.session->connection(connectionIndex);
    EXPECTS(sender.connection != nullptr);
    EXPECTS(sender.connection == conn);
    
    sender.connection->connectionState(eConnectionState(newState));

    if(newState == CONNECTION_DISCONNECTED) {
      for(const auto& cb: sender.session->mLeaveCb) {
        cb(*sender.session, *sender.connection);
      }
      sender.connection->disconnect();
    }

    if(connectionIndex == HOST_CONNECTION_INDEX && newState == CONNECTION_DISCONNECTED) {
      for(const auto& cb: sender.session->mLeaveCb) {
        cb(*sender.session, *sender.session->selfConnection());
      }
      sender.session->disconnect();
    }
    return true;
  }, NETMSSAGE_OPTION_RELIALBE_IN_ORDER, 1);

  on(uint8_t(NETMSG_OBJECT_CREATE), "object_create", [](NetMessage msg, UDPSession::Sender& sender) {
    if(sender.session->selfIndex() == sender.connection->indexOfSession()) return false;
    net_object_id_t objId;
    net_object_type_t objTypeId;

    msg >> objId >> objTypeId;

    NetObjectManager& objectManager = sender.session->netObjectManager();
    auto objType = objectManager.type(objTypeId);
    net_object_local_object_t* object = objType->receiveCreate(msg);

    Log::logf("create object with id: %u", objId);

    if(object != nullptr) {
      objectManager.createObject(objTypeId, object);
      Log::log("...... and it's net object");

    }

    return true;
  }, NETMSSAGE_OPTION_RELIALBE_IN_ORDER, 2);

  on(uint8_t(NETMSG_OBJECT_DESTROY), "object_destory", [](NetMessage msg, UDPSession::Sender& sender) {
    if(sender.session->selfIndex() == sender.connection->indexOfSession()) return false;
    net_object_id_t objId;
    net_object_type_t objTypeId;

    msg >> objId >> objTypeId;

    NetObjectManager& objectManager = sender.session->netObjectManager();

    NetObject* obj = objectManager.find(objId);
    if(obj == nullptr) {
      Log::logf("try to destory non-existing object with id: %u", objId);
      return false;
    }
    Log::logf("destory net object with id: %u", objId);

    auto objType = objectManager.type(objTypeId);
    objType->receiveDestory(msg, obj->ptr);

    objectManager.destoryObject(obj);

    return true;
  }, NETMSSAGE_OPTION_RELIALBE_IN_ORDER, 2);

  on(uint8_t(NETMSG_OBJECT_UPDATE), "object_update", [](NetMessage msg, UDPSession::Sender& sender) {
    if(sender.session->selfIndex() == sender.connection->indexOfSession()) return false;

    uint16_t updateProcessed;

    msg >> updateProcessed;

    Log::logf("the message has %u objects", updateProcessed);
    while(msg.tellr() < msg.tellw()) {

      net_object_id_t objId;
      Timestamp lastUpdate;
      uint16_t snapshotSize;

      size_t readed = msg.read(&objId, sizeof(net_object_id_t));

      if(readed < sizeof(net_object_id_t)) break;
      
      msg >> lastUpdate >> snapshotSize;


      NetObjectManager& objectManager = sender.session->netObjectManager();
      NetObject* obj = objectManager.find(objId);
      
      if(obj == nullptr) {
        Log::logf("receive object update mseesage for %u, but object does not exist in the manager", objId);

        net_object_snapshot_t* data = (net_object_snapshot_t*)_alloca(snapshotSize);
        msg.read(data, snapshotSize);

        continue;
      }

      auto objType = objectManager.type(obj->type);

      NetObject::snapshot_t& snapshot = obj->latestSnapshot();
      objType->receiveSync(snapshot.data, msg);
      if(lastUpdate.stamp < snapshot.lastUpdate) {
        // Log::log("receive expired object update, discard");
        continue;
      }

      snapshot.lastUpdate = lastUpdate.stamp;
      // bool isNew = 
      // sender.connection->updateView(obj, snapshot.data, lastUpdate.stamp);
      // if(isNew) {
      //   objType->applySnapshot(obj->ptr, snapshot, float(sender.session->sessionTimeMs() - lastUpdate.stamp) / 1000.f);
      // }

    }

    return true;
  }, NETMESSAGE_OPTION_DEFAULT);

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
//
// void UDPSession::updateNetObjects() {
//   if(isRunning()) {
//     selfConnection()->netObjectViewCollection().update(mNetObjectManager);
//   }
// }

NetObject::View UDPSession::createView(NetObject& netObject) {
  NetObject::View view;
  view.ref = &netObject;
  // view.snapshot.size = netObjectManager().type(&netObject)->snapshotSize();
  // view.snapshot.data = (net_object_snapshot_t*)malloc(view.snapshot.size);
  view.lastUpdate.stamp = sessionTimeMs();
  return view;
}

NetObject::View UDPSession::cloneView(const NetObject::View& view) {
  NetObject::View newView;
  newView.ref = view.ref;
  newView.lastUpdate = view.lastUpdate;
  // newView.snapshot.size = view.snapshot.size;
  //
  // newView.snapshot.data = (net_object_snapshot_t*)malloc(view.snapshot.size);
  // memcpy(newView.snapshot.data, view.snapshot.data, view.snapshot.size);

  return newView;
}

void UDPSession::destoryView(NetObject::View& view) {
  view.ref = nullptr;
  // free(view.snapshot.data);
  // view.snapshot.size = 0;
}

void UDPSession::registerToConnections(const NetObject::View& view) {
  for(UDPConnection& connection: mConnections) {
    if (!connection.valid()) continue;
    connection.netObjectViewCollection().add(cloneView(view));
  }
}

void UDPSession::unregisterFromConnections(const NetObject& obj) {
  for(UDPConnection& connection: mConnections) {
    if (!connection.valid()) continue;
    NetObject::View view = connection.netObjectViewCollection().remove(&obj);
    destoryView(view);
  }
}

bool UDPSession::onJoinAccepted(NetMessage msg, UDPSession::Sender&) {
  uint8_t index;
  msg >> index;

  uint64_t hostTime;
  msg >> hostTime;

  mDesiredClientMilliSec = hostTime;
  mCurrentClientMilliSec = hostTime;

  UDPConnection& tempConnection = *selfConnection();
  mConnections[index] = std::move(tempConnection);
  mConnections[index].set(*this, index, mConnections[index].addr());
  new(&tempConnection) UDPConnection();

  mSelfIndex = index;
  selfConnection()->connectionState(CONNECTION_CONNECTED);
  ENSURES(selfConnection()->connected());
  

  return true;
}

std::optional<uint8_t> UDPSession::aquireNextAvailableConnection() {
  for (uint8_t i = 1; i < mConnections.size(); ++i) {
    UDPConnection& conn = mConnections[i];
    if(!conn.valid()) {
      new(&conn) UDPConnection();

      return i;
    }
  }

  return std::nullopt;
}

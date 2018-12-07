#include "NetObject.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Net/UDPSession.hpp"
#include "Engine/Debug/Log.hpp"

void NetObject::ViewCollection::add(View&& view) {
  for(View& v: mViews) {
    if(v.ref == view.ref) { return; };
  }
  mViews.push_back(view);
}

NetObject::View NetObject::ViewCollection::remove(const NetObject* ref) {
  for(size_t i = mViews.size() - 1; i < mViews.size(); --i) {
    if(ref == mViews[i].ref) {
      std::swap(mViews[i], mViews.back());
      View view = std::move(mViews.back());
      mViews.pop_back();

      return view;
    } 
  }

  ENSURES(0);
}

NetObject::View* NetObject::ViewCollection::find(NetObject* ref) {
  for(View& v: mViews) {
    if(v.ref == ref) {
      return &v;
    }
  }

  return nullptr;
}

void NetObject::ViewCollection::update(const NetObjectManager& manager) {
  for(View& view: mViews) {
    auto hook = manager.type(view.ref);
    // hook->fillSnapshot(view.snapshot.data, view.ref->ptr);
    view.lastUpdate.stamp = manager.session()->sessionTimeMs();
  }
}

void NetObject::ViewCollection::update(NetObject* ref, const NetObjectManager& manager) {
  View& view = *find(ref);
  auto hook = manager.type(view.ref);
  // hook->fillSnapshot(view.snapshot.data, view.ref->ptr);
  view.lastUpdate.stamp = manager.session()->sessionTimeMs();
}

void NetObject::ViewCollection::reset() {}

void NetObject::ViewCollection::sort() {
  std::sort(mViews.begin(), mViews.end());
}

NetObject::ViewCollection::~ViewCollection() {
}

NetObjectManager::NetObjectManager() {
  mTypeLookup.fill(nullptr);
}

void NetObjectManager::init(UDPSession& session) {
  mSession = &session;

  for(NetObjectDefinition* typeLookup: mTypeLookup) {
    EXPECTS(typeLookup == nullptr);
  }
}

void NetObjectManager::subscribe(net_object_type_t id, NetObjectDefinition* def) {
  EXPECTS(mTypeLookup[id] == nullptr);
  mTypeLookup[id] = def;
}

void NetObjectManager::sync(net_object_type_t type, net_object_local_object_t* obj) {
  NetObject* netObject = createObject(type, obj);
  Log::logf("host create net object: %u", netObject->id);
  NetObject::View view = mSession->createView(*netObject);
  mSession->registerToConnections(view);
  notifySync(netObject);
}

void NetObjectManager::desync(net_object_local_object_t* obj) {
  NetObject* netObject = find(obj);
  Log::logf("desync object: %u", netObject->id);
  if(netObject != nullptr) {
    desync(netObject);
  } else {
    Log::log("failed to find object");
  }
}

void NetObjectManager::desync(NetObject* netObject) {
  
  NetObjectDefinition* def = mTypeLookup[netObject->type];

  NetMessage destory(NETMSG_OBJECT_DESTROY);

  destory << netObject->id
          << netObject->type;

  def->sendDestory(destory, netObject->ptr);

  mSession->sendOthers(destory);

  bool result = destoryObject(netObject);

  mSession->unregisterFromConnections(*netObject);

}

void NetObjectManager::notifySync(const NetObject* netObject) {
  NetObjectDefinition* def = mTypeLookup[netObject->type];
  Log::logf("notify create net object: %u", netObject->id);
  
  NetMessage create(NETMSG_OBJECT_CREATE);

  create << netObject->id
         << netObject->type;

  def->sendCreate(create, netObject->ptr);

  mSession->sendOthers(create);
}


void NetObjectManager::notifySync(const NetObject* netObject, UDPConnection& connection) {
  NetObjectDefinition* def = mTypeLookup[netObject->type];
  Log::logf("notify create net object: %u", netObject->id);
  
  NetMessage create(NETMSG_OBJECT_CREATE);

  create << netObject->id
         << netObject->type;

  // wait.. what if sendcreate has side effect?
  def->sendCreate(create, netObject->ptr);

  EXPECTS(connection.owner() == mSession);
  connection.send(create);
}

void NetObjectManager::notifySync(net_object_local_object_t* obj, UDPConnection& connection) {
  NetObject* netObject = find(obj);
  if(netObject == nullptr) {
    Log::log("try to sync local object, which is not in the net object look up table");
    return;
  }

  notifySync(netObject, connection);
}

net_object_id_t NetObjectManager::acquireNextUsableId() {
  EXPECTS(mNextUsableId < UINT16_MAX);
  return mNextUsableId++;
}

void NetObjectManager::applySnapshots() {
  uint64_t curTime = mSession->sessionTimeMs();
  for(auto [_, object]: mObjectPtrLookup) {
    auto proxy = type(object->type);
    proxy->applySnapshot(object->ptr, object->latestSnapshot().data, float(curTime - object->latestSnapshot().lastUpdate) / 1000.f);
  }
}

void NetObjectManager::updateSnapshots() {
  uint64_t curTime = mSession->sessionTimeMs();
  for(auto [_, object]: mObjectPtrLookup) {
    auto proxy = type(object->type);
    NetObject::snapshot_t& snapshot = object->latestSnapshot();
    proxy->fillSnapshot(snapshot.data, object->ptr);
    snapshot.lastUpdate = curTime;
  }
}

NetObject* NetObjectManager::createObject(net_object_type_t type, net_object_local_object_t* localObject) {
  NetObject* obj = new NetObject();

  obj->id = acquireNextUsableId();
  obj->type = type;
  obj->ptr = localObject;

  mObjectIdLookup[obj->id] = obj;
  mObjectPtrLookup[obj->ptr] = obj;

  auto objType = this->type(type);
  obj->mLatestsnapshot.size = objType->snapshotSize();
  obj->mLatestsnapshot.data = (net_object_snapshot_t*)malloc(obj->mLatestsnapshot.size);
  this->type(type)->fillSnapshot(obj->mLatestsnapshot.data, obj->ptr);

  return obj;
}

bool NetObjectManager::destoryObject(NetObject* obj) {
  if(obj == nullptr) return false;
  mObjectIdLookup.erase(obj->id);
  mObjectPtrLookup.erase(obj->ptr);

  Log::logf("net object with id %u destroyed", obj->id);
  free(obj->mLatestsnapshot.data);
  
  delete obj;
  return true;
}

NetObjectManager::NetObjectDefinition* NetObjectManager::type(net_object_type_t type) const {
  return mTypeLookup[type];
}

NetObjectManager::NetObjectDefinition* NetObjectManager::type(NetObject* object) const {
  return type(object->type);
}

NetObject* NetObjectManager::find(net_object_local_object_t* obj) {
  auto kv = mObjectPtrLookup.find(obj);
  if(kv == mObjectPtrLookup.end()) return nullptr;

  return kv->second;
}

NetObject* NetObjectManager::find(net_object_id_t id) {
  auto kv = mObjectIdLookup.find(id);
  if(kv == mObjectIdLookup.end()) return nullptr;

  return kv->second;
}

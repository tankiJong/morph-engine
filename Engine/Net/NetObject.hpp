#pragma once
#include "Engine/Core/common.hpp"
#include <map>
#include <vector>
#include "Engine/Core/Time/Time.hpp"

class NetMessage;
class UDPSession;
class UDPConnection;
class NetObjectManager;
using net_object_type_t = uint8_t;
using net_object_id_t = uint32_t;
struct net_object_local_object_t {};
struct net_object_snapshot_t {};

class NetObject {
  friend class NetObjectManager;
public:
  struct snapshot_t {
    net_object_snapshot_t* data = nullptr;
    uint16_t size = 0;
    uint64_t lastUpdate;
  };

  net_object_id_t id = 0;
  net_object_type_t  type = 0;
  net_object_local_object_t* ptr = nullptr;

  snapshot_t& latestSnapshot() { return mLatestsnapshot; };
  struct View {
    NetObject* ref;
    Timestamp lastUpdate;

    // View() = default;
    // View(View&& view) = default;
    // View& operator=(View&& view) = default;
    //
    // View(const View& view) = delete;
    // View& operator=(const View& view) = delete;

    bool operator<(const View& rhs) { return lastUpdate.stamp < rhs.lastUpdate.stamp;}
  };

  class ViewCollection {
  public:
    void add(View&& view);
    View remove(const NetObject* ref);

    View* find(NetObject* ref);

    void update(const NetObjectManager& manager);
    void update(NetObject* ref, const NetObjectManager& manager);

    void reset();

    void sort();
    span<View> views() { return mViews;}

    ~ViewCollection();
  protected:
    std::vector<View> mViews;
    
  };
protected:
  snapshot_t mLatestsnapshot;
};


class NetObjectManager {
public:
  class NetObjectDefinition {
    friend class NetObjectManager;
    friend class UDPSession;
  public:
    virtual ~NetObjectDefinition() = default;

    virtual size_t snapshotSize() = 0;
    virtual void sendCreate(NetMessage& msg, const net_object_local_object_t* obj) = 0;
    virtual net_object_local_object_t* receiveCreate(NetMessage& msg) = 0;
                  
    virtual void  sendDestory(NetMessage& msg, const net_object_local_object_t* obj)= 0;
    virtual void  receiveDestory(NetMessage& msg, net_object_local_object_t* obj) = 0;
                  
    virtual void  sendSync(NetMessage& msg, const net_object_snapshot_t* obj) = 0;
    virtual void  receiveSync(net_object_snapshot_t* snapshot, NetMessage& msg) = 0;
                  
    virtual void  fillSnapshot(net_object_snapshot_t* snapshot, const net_object_local_object_t* obj) = 0;
    virtual void  applySnapshot(net_object_local_object_t* obj, const net_object_snapshot_t* snapshot, float snapshotAgeSec) = 0;
  };
  NetObjectManager();

  void init(UDPSession& session);

  template<typename T>
  void subscribe(net_object_type_t id) {
    static_assert(std::is_base_of_v<NetObjectDefinition, T>, "T has to be derived from `NetObjectDefinition`");
    subscribe(id, new T());
  }

  void subscribe(net_object_type_t id, NetObjectDefinition* def);

  void sync(net_object_type_t type, net_object_local_object_t* obj);
  void desync(net_object_local_object_t* obj);
  void desync(NetObject* obj);

  void notifySync(const NetObject* netObject);
  void notifySync(const NetObject* netObject, UDPConnection& connection);
  void notifySync(net_object_local_object_t* obj, UDPConnection& connection);

  NetObject* createObject(net_object_type_t type, net_object_local_object_t* localObject);
  bool destoryObject(NetObject* obj);

  NetObject* find(net_object_local_object_t* obj);
  NetObject* find(net_object_id_t id);

  NetObjectDefinition* type(net_object_type_t type) const;
  NetObjectDefinition* type(NetObject* obj) const;
  net_object_id_t acquireNextUsableId();

  UDPSession* session() const { return mSession; }

  void applySnapshots();

  void updateSnapshots();

protected:
  UDPSession* mSession = nullptr;
  std::map<net_object_local_object_t*, NetObject*> mObjectPtrLookup;
  std::map<uint16_t, NetObject*> mObjectIdLookup;
  std::array<NetObjectDefinition*, UINT8_MAX> mTypeLookup;
  net_object_id_t mNextUsableId = 0;
};


template<typename ObjType, typename SnapshotType = ObjType>
class NetObjectHook: public NetObjectManager::NetObjectDefinition {

public:
  virtual ~NetObjectHook() override = default;

  virtual void onSendCreate(NetMessage& msg, const ObjType* ptr) = 0;
  virtual ObjType*   onReceiveCreate(NetMessage& msg) = 0;

  virtual void onSendDestory(NetMessage& msg, const ObjType* obj) = 0;
  virtual void onReceiveDestory(NetMessage& msg, ObjType* obj) = 0;
               
  virtual void onSendSync(NetMessage& msg, const SnapshotType* obj) = 0;
  virtual void onReceiveSync(SnapshotType* snapshot, NetMessage& msg) = 0;
               
  virtual void onFillSnapshot(SnapshotType* snapshot, const ObjType* obj) = 0;
  virtual void onApplySnapshot(ObjType* obj, const SnapshotType* snapshot, float snapshotAgeSec) = 0;

private:
  size_t snapshotSize() override {
    return sizeof(SnapshotType);
  }

  void sendCreate(NetMessage& msg, const net_object_local_object_t* ptr) override {
    onSendCreate(msg, (const ObjType*) ptr);
  };

  net_object_local_object_t* receiveCreate(NetMessage& msg) override {
    return (net_object_local_object_t*)onReceiveCreate(msg);
  };

  void sendDestory(NetMessage& msg, const net_object_local_object_t* obj) override {
    onSendDestory(msg, (ObjType*)obj);
  };

  void receiveDestory(NetMessage& msg, net_object_local_object_t* obj) override {
    onReceiveDestory(msg, (ObjType*)obj);
  };

  void sendSync(NetMessage& msg, const net_object_snapshot_t* obj) override {
    onSendSync(msg, (const SnapshotType*)obj);
  };

  void receiveSync(net_object_snapshot_t* snapshot, NetMessage& msg) override {
    onReceiveSync((SnapshotType*)snapshot, msg);
  };

  void fillSnapshot(net_object_snapshot_t* snapshot, const net_object_local_object_t* obj) override {
    onFillSnapshot((SnapshotType*)snapshot, (const ObjType*)obj);
  };

  void applySnapshot(net_object_local_object_t* obj, const net_object_snapshot_t* snapshot, float snapshotAgeSec) override {
    onApplySnapshot((ObjType*)obj, (const SnapshotType*)snapshot, snapshotAgeSec);
  };
};
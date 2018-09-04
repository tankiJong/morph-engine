#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/NetAddress.hpp"
#include <functional>
#include "Engine/Core/Endianness.hpp"
#include "Engine/Net/TCPSocket.hpp"
#include "Engine/Async/Thread.hpp"
#include "Engine/Core/BytePacker.hpp"
#include <mutex>

class RemoteConsole {
public:
  enum eRemoteConsoleCharacter {
    CHARACTER_UNKNOWN = -1,
    CHARACTER_HOST,
    CHARACTER_CLIENT,
  };

  struct Instr {
    bool isEcho;
    char content[256];

    static constexpr eEndianness INSTR_ENDIANNESS = ENDIANNESS_BIG;
  };

  struct Connection {
    TCPSocket socket;
    BytePacker packer;

    Connection(): packer(512, Instr::INSTR_ENDIANNESS) {}
    Connection(TCPSocket&& sock): socket(std::move(sock)), packer(512, Instr::INSTR_ENDIANNESS) {}
    Connection(Connection&& c): socket(std::move(c.socket)), packer(std::move(c.packer)) {}

    Connection& operator=(Connection&& c) {
      socket = std::move(c.socket);
      packer = std::move(c.packer);

      return *this;
    }
  };

  ~RemoteConsole();

  void init();
  void reset();
  bool join(const NetAddress& host);
  bool host(uint16_t port = REMOTE_CONSOLE_HOST_PORT);

  bool ready() const;

  void issue(uint index, bool isEcho, const char* cmd);
  void issue(uint index, const Instr& instr);
  void broadcast(bool isEcho, const char* cmd);
  void broadcast(Instr& instr);

  const Connection& connection(uint index) const { return mConnections[index]; }
  static void startup();
  static void shutdown();
  static RemoteConsole& get();
  template<typename T>
  void onReceive(T&& handle) {
    mHandles.push_back(handle);
  }

  static constexpr uint16_t REMOTE_CONSOLE_HOST_PORT = 29283;

protected:
  RemoteConsole();
  void receive(uint index, const Instr& instr) const;
  void flushConnection(uint index, RemoteConsole::Connection& connection);
  void manageConnections();
  eRemoteConsoleCharacter mCharacter = CHARACTER_UNKNOWN;
  std::vector<std::function<void(uint index, const Instr&)>> mHandles;
  std::vector<Connection> mConnections;

  std::mutex mConnectionLock;
  Thread* mSockManageThread = nullptr;
  bool mIsDying = false;
};

#include "RemoteConsole.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Core/BytePacker.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Debug/Console/Command.hpp"

void RemoteConsole::receive(uint index, const Instr& instr) const {
  for(auto& handle: mHandles) {
    handle(index, instr);
  }
}

// message format: [uint16_t: size] [uint8_t: isEcho] [string: data]
void RemoteConsole::flushConnection(uint index, Connection& connection) {
  BytePacker& packer = connection.packer;
  packer.seekr(0, BytePacker::SEEK_DIR_BEGIN);

  // receive length bytes
  if(packer.size() < 2) {
    char buffer[10];
    size_t count = connection.socket.receive(buffer, 2 - packer.size());
    packer.append(buffer, count);
    return;
  }

  uint16_t size;
  packer >> size;

  // receive content
  if(packer.size() < size + 2) {
    char buffer[0xffff];
    size_t count = connection.socket.receive(buffer, size - packer.size() + 2);
    packer.append(buffer, count);
    return;
  }

  // decode string
  Instr instr;
  packer >> instr.isEcho;
  packer.read(instr.content, std::size(instr.content));

  receive(index, instr);

  packer.clear();
}

void RemoteConsole::manageConnections() {
  if (mCharacter == CHARACTER_UNKNOWN)return;
  if (mCharacter == CHARACTER_CLIENT) {
    if (!mConnections[0].socket.valid()) {
      reset();
    }
    return;
  }

  // as host:

  // 0 position is self
  for(auto begin = mConnections.rbegin(); begin != mConnections.rend() - 1; ++begin) {
    if(!begin->socket.valid()) {
      Connection& back = mConnections.back();
      *begin = std::move(back);
      mConnections.pop_back();
    }
  }

  TCPSocket* client = mConnections[0].socket.accept();
  if (client != nullptr) {
    Log::tagf("remote console", "client %s join the session.", client->address().toString());
    mConnections.emplace_back(std::move(*client));
  }

  SAFE_DELETE(client);
}

RemoteConsole::RemoteConsole() {
  BytePacker packer;

  size_t a = 8675309;

  packer.write(a);
  packer.read(a);

  EXPECTS(a == 8675309);
  mSockManageThread = new Thread("Remote Command Update", [this]() {

    init();

    while(true) {
      if (ready()) {
        std::scoped_lock lock(mConnectionLock);

        manageConnections();

        for(uint i = 0; i < mConnections.size(); i++) {
          flushConnection(i, mConnections[i]);
        }

        for(Connection& connection: mConnections) {
        }
      } else {
        reset();
        init();
      }
      CurrentThread::yield();
    }
  });
}

RemoteConsole::~RemoteConsole() {
  mIsDying = true;
  mSockManageThread->join();
  SAFE_DELETE(mSockManageThread);
  reset();
}

void RemoteConsole::init() {
  bool re = join(NetAddress::local(REMOTE_CONSOLE_HOST_PORT));
  if (!re) {
    re = host();
    ENSURES(re);
  }
}

void RemoteConsole::reset() {
  std::scoped_lock lock(mConnectionLock);
  mConnections.clear();
  mCharacter = CHARACTER_UNKNOWN;
}

bool RemoteConsole::join(const NetAddress& host) {
  EXPECTS(mCharacter == CHARACTER_UNKNOWN);
  EXPECTS(mConnections.empty());

  std::scoped_lock lock(mConnectionLock);
  Connection& c = mConnections.emplace_back();
  bool re = c.socket.connect(host);
  if(re) {
    mCharacter = CHARACTER_CLIENT;
    Log::tagf("remote console", "Running in client mode");
  } else {
    mConnections.pop_back();
  }

  return re;
}

bool RemoteConsole::host(uint16_t port) {
  EXPECTS(mCharacter == CHARACTER_UNKNOWN);
  EXPECTS(mConnections.empty());

  std::scoped_lock lock(mConnectionLock);
  Connection& c = mConnections.emplace_back();
  bool re = c.socket.listen(port);
  if(re) {
    mCharacter = CHARACTER_HOST;
    Log::tagf("remote console", "Running in host mode");
  } else {
    std::scoped_lock lock(mConnectionLock);
    mConnections.pop_back();
  }

  return re;
}

bool RemoteConsole::ready() const {
  if (mCharacter == CHARACTER_UNKNOWN) return false;
  if (mConnections.empty()) return false;
  if (!mConnections[0].socket.valid()) return false;
  return true;
}

void RemoteConsole::issue(uint index, bool isEcho, const char* cmd) {

  if (mCharacter == CHARACTER_UNKNOWN) {
    Log::tagf("Remote Command", "Fail to issue remote command, since the remote command is not ready yet");
    return;
  }
  if(index >= mConnections.size()) {
    Log::tagf("Remote Command", "Fail to issue remote command, index is invalid");
    return;
  }

  Connection& c = mConnections[index];
  BytePacker packer(1024, Instr::INSTR_ENDIANNESS);

  isEcho >> packer;
  packer.write(cmd);

  uint16_t size = (uint16_t)packer.size();
  uint16_t sendSize = size;
  
  toEndianness(Instr::INSTR_ENDIANNESS, sendSize);

  c.socket.send(sendSize);
  c.socket.send(packer.data(), size);
}

void RemoteConsole::issue(uint index, const Instr& instr) {
  issue(index, instr.isEcho, instr.content);
}

void RemoteConsole::broadcast(bool isEcho, const char* cmd) {
  if(mCharacter == CHARACTER_UNKNOWN) {
    Log::tagf("remote console", "tried to send command while remote console is not ready yet.");
    return; 
  }

  if(mCharacter == CHARACTER_HOST & mConnections.size() > 1) {
    for(uint i = 1; i < mConnections.size(); i++) {
      issue(i, isEcho, cmd);
    }
    return;
  }

  if(mCharacter == CHARACTER_CLIENT) {
    issue(0, isEcho, cmd);
    return;
  }
}

static RemoteConsole* gRemoteConsole = nullptr;

void RemoteConsole::startup() {
  gRemoteConsole = new RemoteConsole();
}

void RemoteConsole::shutdown() {
  SAFE_DELETE(gRemoteConsole);
}

RemoteConsole& RemoteConsole::get() {
  EXPECTS(gRemoteConsole != nullptr);
  return *gRemoteConsole;
}

COMMAND_REG("rcb", "[command: string]", "")(Command& cmd) {
  std::string c = cmd.arg<0>();

  gRemoteConsole->broadcast(false, c.c_str());

  return true;
};
#include "RemoteConsole.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Core/BytePacker.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Debug/Console/Command.hpp"
#include "Engine/Net/NetAddress.hpp"
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
  if (!ready())return;
  if (mServiceState == STATE_JOIN) {
    if (!mConnections[0].socket.opened()) {
      reset();
    }
    return;
  }

  // as host:

  // 0 position is self
  for(auto begin = mConnections.rbegin(); begin != mConnections.rend() - 1; ++begin) {
    if(!begin->socket.opened()) {
      Log::tagf("remote console", "client %s leave the session.", begin->socket.address().toString());
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

  mSockManageThread = new Thread("Remote Command Update", [this]() {

    init();

    while(!mIsDying) {
      if (ready()) {
        std::scoped_lock lock(mConnectionLock);

        manageConnections();

        for(uint i = mServiceState == STATE_JOIN ? 0 : 1; i < mConnections.size(); i++) {
          flushConnection(i, mConnections[i]);
        }

        for(Connection& connection: mConnections) {
        }
      } else {
        if (mServiceState == STATE_TRY_JOIN || mServiceState == STATE_TRY_HOST) continue;
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
    host();
  }
}

void RemoteConsole::reset() {
  std::scoped_lock lock(mConnectionLock);
  mConnections.clear();
  mServiceState = STATE_INIT;
}

bool RemoteConsole::join(const NetAddress& host) {
  std::scoped_lock lock(mConnectionLock);
  if (mServiceState != STATE_INIT) return false;
  EXPECTS(mConnections.empty());
  mServiceState = STATE_TRY_JOIN;

  Connection& c = mConnections.emplace_back();
  bool re = c.socket.connect(host);
  c.socket.unsetOption(SOCKET_OPTION_BLOCKING);

  if(re) {
    mServiceState = STATE_JOIN;
    Log::tagf("remote console", "Running in client mode");
  } else {
    mConnections.pop_back();
    mServiceState = STATE_INIT;
  }

  return re;
}

bool RemoteConsole::host(uint16_t port) {
  std::scoped_lock lock(mConnectionLock);
  if (mServiceState != STATE_INIT) return false;
  EXPECTS(mConnections.empty());
  mServiceState = STATE_TRY_HOST;

  Connection& c = mConnections.emplace_back();
  bool re = c.socket.listen(port);

  if(re) {
    mServiceState = STATE_HOST;
    c.socket.unsetOption(SOCKET_OPTION_BLOCKING);
    Log::tagf("remote console", "Running in host mode");
  } else {
    mConnections.pop_back();
    mServiceState = STATE_INIT;
  }

  return re;
}

bool RemoteConsole::ready() const {
  if (mServiceState != STATE_HOST && mServiceState != STATE_JOIN) 
    return false;
  if (mConnections.empty()) 
    return false;
  if (!mConnections[0].socket.opened()) 
    return false;
  return true;
}

void RemoteConsole::issue(uint index, bool isEcho, const char* cmd) {

  if (!ready()) {
    Log::tagf("Remote Command", "Fail to issue remote command, since the remote command is not ready yet");
    return;
  }
  if(index >= mConnections.size()) {
    Log::tagf("Remote Command", "Fail to issue remote command, index is out of range");
    return;
  }

  if(mServiceState == STATE_HOST && index == 0) {
    Log::tagf("Remote Command", "Fail to issue remote command, 0 is not valid connection in host mode");
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
  if(!ready()) {
    Log::tagf("remote console", "tried to send command while remote console is not ready yet.");
    return; 
  }

  if(mServiceState == STATE_HOST && mConnections.size() > 1) {
    for(uint i = 1; i < mConnections.size(); i++) {
      issue(i, isEcho, cmd);
    }
    return;
  }

  if(mServiceState == STATE_JOIN) {
    issue(0, isEcho, cmd);
    return;
  }
}

void RemoteConsole::echo(std::string content) {
  if (!mEnableEcho) return;

  Console::log(content, Rgba::white);
}

void RemoteConsole::toggleEcho(bool e) {
  mEnableEcho = e;
}

void RemoteConsole::printState() const {
  if(!ready()) {
    Log::tagf("remote console", "Remote Console is not ready yet.");
    return;
  }

  EXPECTS(mServiceState == STATE_HOST || mServiceState == STATE_JOIN);
  Log::tagf("remote console", "remote console running in %s mode", mServiceState == STATE_HOST ? "Host" : "Client");
  Log::tagf("remote console", "Current connections: ");

  uint start = mServiceState == STATE_HOST ? 1 : 0;

  while(start < mConnections.size()) {
    Log::tagf("remote console", "%s", mConnections[start].socket.address().toString());
    start++;
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


COMMAND_REG("rc", "[index: uint = 0][command: string]", "execute a command on a connection")(Command& cmd) {
  uint index = cmd.arg<0, uint>();
  std::string command;
  if(cmd.arg<0>()[0] != '0' && index == 0) {
    command = cmd.arg<0>();
  } else {
    command = cmd.arg<1>();
  }

  gRemoteConsole->issue(index, false, command.c_str());
  return true;
}

COMMAND_REG("rcb", "[command: string]", "Execute a command on everyone except me.")(Command& cmd) {
  std::string c = cmd.arg<0>();

  gRemoteConsole->broadcast(false, c.c_str());

  return true;
};

COMMAND_REG("rca", "[command: string]", "Execute a command on everyone.")(Command& cmd) {
  std::string c = cmd.arg<0>();

  Console::Get()->exec(c);
  gRemoteConsole->broadcast(false, c.c_str());
  return true;
};

COMMAND_REG("rc_join", "[address:port]", "leave current service and try to join another service.")(Command& cmd) {
  gRemoteConsole->reset();

  NetAddress addr = cmd.arg<0>();

  return gRemoteConsole->join(addr);
}

COMMAND_REG("rc_host", "[port: uint]", "start hosting on the port.")(Command& cmd) {
  gRemoteConsole->reset();

  uint port = cmd.arg<0, uint>();

  return gRemoteConsole->host(port);
}

COMMAND_REG("rc_list", "", "log the remote console service status.")(Command& cmd) {
  gRemoteConsole->printState();
  return true;
}

COMMAND_REG("rc_echo", "[enabled: bool]", "Enable/disable the output from the remote console.")(Command& cmd) {
  gRemoteConsole->toggleEcho(cmd.arg<0, bool>());
  return true;
}
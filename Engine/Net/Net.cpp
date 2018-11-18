#include "Engine/Net/Net.hpp"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Net/TCPSocket.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Async/Thread.hpp"
#include "Engine/Debug/Console/Command.hpp"

bool Net::startup() {
  WORD version = MAKEWORD(2, 2);
  WSADATA data;
  int32_t error = ::WSAStartup(version, &data);

  ENSURES(error == 0);

  return true;
}

bool Net::shutdown() {
  ::WSACleanup();
  return true;
}

COMMAND_REG("self_ip", "", "")(Command&) {
  // Log::tagf("net", "Local IP: %s", NetAddress::local().toString());
  return true;
}

COMMAND_REG("a1_test_connect", "[IP:HOST][MESSAGE]", "")(Command& cmd) {
  char rc[256];
  std::string ss = cmd.arg<1, std::string>();
  NetAddress address = cmd.arg<0, std::string>();
  // while(true) {
    TCPSocket s;
    s.connect(address);
    s.send(ss.data(), ss.size());
    int re = s.receive(rc, 256u);
    if(re > 0) {
      // Log::tagf("net", rc);
    }

    // CurrentThread::sleep(100);
  // }
  return true;
}

COMMAND_REG("a1_test_server", "PORT", "")(Command& cmd) {
  static bool running = false;
  static Thread* hostThread;

  if(running) {
    running = false;
    hostThread->join();
    delete hostThread;
    return true;
  }

  uint16_t port = (uint16_t)cmd.arg<0, uint>();

  hostThread = new Thread([port]() {
    TCPSocket host;

    host.listen(port);

    running = true;

    // Log::tagf("net", "Host is listening on %s", host.address().toString());
    while(running) {
      TCPSocket* sock = host.accept();
      if (sock == nullptr) {
        continue;
      }

      // Log::tagf("net", "Connection built with %s", sock->address().toString());

      byte_t buf[1024];
      sock->receive(buf, 1024 - 1u);
      // Log::tagf("net", "Received from [%s]: %s", sock->address().toString(), buf);

      sock->send("OK", 2);

      delete sock;
    }
    
  });
}
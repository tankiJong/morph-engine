#include "TCPSocket.hpp"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"

TCPSocket::TCPSocket() {
  mHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ENSURES(mHandle != INVALID_SOCKET);
}

TCPSocket::~TCPSocket() {
  close();
}

bool TCPSocket::listen(uint16_t port, uint maxQueued) {
  NetAddress local = NetAddress::any(port);
  bind(local);

  int result = ::listen(mHandle, maxQueued);
  if(result != 0) {
    close();
    Log::tagf("net", "fail to listen at %u", port);
    return false;
  }

  return true;
}

owner<TCPSocket*> TCPSocket::accept() {
  sockaddr_storage client;
  int clientAddrLen = sizeof(sockaddr_storage);

  SOCKET socket = ::accept(mHandle, (sockaddr*)&client, &clientAddrLen);
  if (socket == INVALID_SOCKET) {
    int re = WSAGetLastError();
    return nullptr;
  }

  TCPSocket* sock = new TCPSocket();

  sock->mHandle = socket;
  sock->mAddress.fromSockaddr((sockaddr&)client);

  return sock;
}

bool TCPSocket::bind(const NetAddress& addr) {
  mAddress = addr;

  sockaddr_storage saddr;
  int len;
  addr.toSockaddr((sockaddr&)saddr, len);

  int result = ::bind(mHandle, (sockaddr*)&saddr, len);

  if(result == SOCKET_ERROR) {
    close();
    Log::tagf("net", "fail to bind to %s.", addr.toString());
    return false;
  }

  return true;
}

bool TCPSocket::connect(const NetAddress& addr) {
  sockaddr_storage saddr;
  int len;
  addr.toSockaddr((sockaddr&)saddr, len);

  int result = ::connect(mHandle, (sockaddr*)&saddr, len);

  if(result == SOCKET_ERROR) {
    Log::tagf("net", "could not connect %s", addr.toString());
    return false;
  }

  Log::tagf("net", "connected to %s", addr.toString());
  return true;
}

void TCPSocket::send(void* data, size_t size) {
  ::send(mHandle, (char*)data, (int)size, 0);
}

size_t TCPSocket::receive(void* buf, size_t max) {
  int re = ::recv(mHandle, (char*)buf, max, 0);
  if(re == SOCKET_ERROR) {
    int re = WSAGetLastError();
    Log::tagf("net", "Receive data failed");
    return 0;
  }

  return re;
}

void TCPSocket::close() {
  if (mHandle == INVALID_SOCKET) {
    return;
  }
  int result = ::closesocket(mHandle);
  if(result == SOCKET_ERROR) {
    Log::tagf("net", "fail to close socket");
    return;
  } else {
    Log::tagf("net", "Connection to [%s] closed", mAddress.toString());
  }
  mHandle = INVALID_SOCKET;
}

bool TCPSocket::isClosed() {
  return mHandle == INVALID_SOCKET;
}

bool TCPSocket::valid() const {
  return mHandle != INVALID_SOCKET;
}

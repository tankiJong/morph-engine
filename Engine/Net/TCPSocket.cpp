#include "TCPSocket.hpp"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"


TCPSocket::TCPSocket() {
  mHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in addr;
  int len = sizeof(sockaddr_in);
  getsockname(mHandle, (sockaddr*)&addr, &len);

  mAddress.fromSockaddr((sockaddr&)addr);
  
  ENSURES(mHandle != INVALID_SOCKET);
}

TCPSocket::TCPSocket(TCPSocket&& socket) {
  mHandle = socket.mHandle;
  mAddress = socket.mAddress;

  socket.mHandle = INVALID_SOCKET;
  socket.mAddress = NetAddress();
}

TCPSocket& TCPSocket::operator=(TCPSocket&& rhs) {
  mHandle = rhs.mHandle;
  mAddress = rhs.mAddress;

  rhs.mHandle = INVALID_SOCKET;
  rhs.mAddress = NetAddress();

  
  return *this;
}

TCPSocket::~TCPSocket() {
  close();
}

bool TCPSocket::listen(uint16_t port, uint maxQueued) {
  NetAddress local = NetAddress::any(port);
  bind(local);

  int result = ::listen(mHandle, maxQueued);
  if(result != 0) {
    LOG_FATAL_SOCK_ERROR();
    close();
    Log::tagf("net", "fail to listen at %u", port);
    return false;
  } else {
    Log::tagf("net", "start listening at %u", port);

  }

  return true;
}

owner<TCPSocket*> TCPSocket::accept() {
  sockaddr_storage client;
  int clientAddrLen = sizeof(sockaddr_storage);

  // set to non-blocking for accept...
  u_long nonBlocking = 1;
  ::ioctlsocket(mHandle, FIONBIO, &nonBlocking);

  SOCKET socket = ::accept(mHandle, (sockaddr*)&client, &clientAddrLen);
  if (socket == INVALID_SOCKET) {
    LOG_FATAL_SOCK_ERROR();
    return nullptr;
  }

  TCPSocket* sock = new TCPSocket();

  sock->mHandle = socket;
  sock->mAddress.fromSockaddr((sockaddr&)client);
  return sock;
}

bool TCPSocket::bind(const NetAddress& addr) {

  sockaddr_storage saddr;

  int len;
  addr.toSockaddr((sockaddr&)saddr, len);

  int result = ::bind(mHandle, (sockaddr*)&saddr, len);

  if(result == SOCKET_ERROR) {
    LOG_FATAL_SOCK_ERROR();
    close();
    Log::tagf("net", "fail to bind to %s.", addr.toString());
    return false;
  } else {
    mAddress = addr;
  }

  return true;
}

bool TCPSocket::connect(const NetAddress& addr) {
  sockaddr_storage saddr;
  int len;
  addr.toSockaddr((sockaddr&)saddr, len);

  int result = ::connect(mHandle, (sockaddr*)&saddr, len);

  mAddress.fromSockaddr((sockaddr&)saddr);

  if(result == SOCKET_ERROR) {
    LOG_FATAL_SOCK_ERROR();
    close();
    Log::tagf("net", "could not connect %s", addr.toString());
    return false;
  }
  
  Log::tagf("net", "connected to %s", addr.toString());
  return true;
}

void TCPSocket::send(const void* data, size_t size) {
  ::send(mHandle, (char*)data, (int)size, 0);
}

size_t TCPSocket::receive(void* buf, size_t max) {
  // set to non-blocking for recv...
  u_long nonBlocking = 1;
  ::ioctlsocket(mHandle, FIONBIO, &nonBlocking);

  int re = ::recv(mHandle, (char*)buf, max, 0);
  if(re == SOCKET_ERROR) {
    // int re = WSAGetLastError();
    // Log::tagf("net", "Receive data failed");
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

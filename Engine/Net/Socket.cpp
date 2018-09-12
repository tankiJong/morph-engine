#include "Socket.hpp"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"


Socket::Socket() {
  mHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in addr;
  int len = sizeof(sockaddr_in);
  getsockname(mHandle, (sockaddr*)&addr, &len);

  mAddress.fromSockaddr((sockaddr&)addr);

  ENSURES(mHandle != INVALID_SOCKET);
}

Socket::~Socket() {
  close();
}

void Socket::setOption(eSocketOption op) {
  if(!is_set(op, SOCKET_OPTION_BLOCKING)) {
    u_long nonBlocking = 1;
    ::ioctlsocket(mHandle, FIONBIO, &nonBlocking);
  } else {
    u_long nonBlocking = 0;
    ::ioctlsocket(mHandle, FIONBIO, &nonBlocking);
  }
  mOptions |= op;
}

bool Socket::close() {
  if (mHandle == INVALID_SOCKET) {
    return true;
  }
  int result = ::closesocket(mHandle);
  if (result == SOCKET_ERROR) {
    Log::tagf("net", "fail to close socket");
    return false;
  } else {
    Log::tagf("net", "Connection to [%s] closed", mAddress.toString());
  }
  mHandle = INVALID_SOCKET;
  return true;
}

bool Socket::valid() const {
  return mHandle != INVALID_SOCKET;
}

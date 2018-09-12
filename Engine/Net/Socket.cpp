#include "Socket.hpp"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"


Socket::Socket() {
	// C4: Do not create socket in constructor;  UDP and TCP sockets have different create settings;
  mHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	// C4: Do not get address in consturctor - different types of sockets treat address differently
	// C4: Listen -> Address I'm listening on;
	// C4: TCP -> Address I'm talking to;
	// C4: UDP -> Address I'm bound to; 
  sockaddr_in addr;
  int len = sizeof(sockaddr_in);
  getsockname(mHandle, (sockaddr*)&addr, &len);

  mAddress.fromSockaddr((sockaddr&)addr);

	// C4: not needed (don't create here)
  ENSURES(mHandle != INVALID_SOCKET);
}

Socket::~Socket() {
  close();
}

// C4: Currently have no way to UNSET an option (need to implement); 
void Socket::setOption(eSocketOption op) {
	// C4TODO: if you don't have socket - just save the option (but don't apply, no socket to apply to)
	// C4: That way, apply once socket is created. 
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

// C4: for things returning bool, prefix with "is_"
// C4: For sockets, I prefer "is_open()" instead of "is_valid()"
bool Socket::valid() const {
  return mHandle != INVALID_SOCKET;
}

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include "UDPSocket.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"

bool UDPSocket::bind(const NetAddress& addr, uint16_t portRange) {
	// C4: If a socket is open and you bind - I would assert and return false;
  close();

  SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  ASSERT_OR_RETURN(sock != INVALID_SOCKET, false);

  sockaddr_storage sockadd;
  int len = sizeof(sockaddr_storage);
  addr.toSockaddr((sockaddr&)sockadd, len);

  int result = ::bind(sock, (sockaddr*)&sockadd, len);

  if( 0 == result ) {
    mHandle = sock;
    mAddress = addr;
    return true;
  }

  return false;
}

size_t UDPSocket::send(const NetAddress& addr, const void* data, size_t byteCount) {
  if(!valid()) {
    return 0;
  }

  sockaddr_storage storage;
  int len;
  addr.toSockaddr((sockaddr&)storage, len);

  SOCKET sock = mHandle;
  int sent = ::sendto(sock, (const char*)data, (int)byteCount, 0, (sockaddr*)&storage, len);

  if(sent > 0) {
    Log::tagf("net", "the sent size is smaller than the actual size");

    return (size_t)sent;
  } else {
		// C4: This looks really weird - turn it into a function that returns bool
    bool re;
    OUT_LOG_FATAL_SOCK_ERROR(re);
    if(re) {
      close();
    }

  }

  return 0;
}

size_t UDPSocket::receive(NetAddress& outAddr, void* buffer, size_t maxSize) {
  if(!valid()) {
    return 0;
  }

  sockaddr_storage storage;
  int len = sizeof(sockaddr_storage);

  SOCKET sock = mHandle;

  int rcvd = ::recvfrom(sock, (char*)buffer, maxSize, 0, (sockaddr*)&storage, &len);

  if(rcvd > 0) {
    outAddr.fromSockaddr((sockaddr&)storage);
    return rcvd;
  } else {
    bool re;
    OUT_LOG_FATAL_SOCK_ERROR(re);
    if(re) {
      close();
    }
  }

  return 0;
}

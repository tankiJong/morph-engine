#include "TCPSocket.hpp"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"


owner<TCPSocket*> TCPSocket::accept() {

  sockaddr_storage client;
  int clientAddrLen = sizeof(sockaddr_storage);

  SOCKET socket = ::accept(mHandle, (sockaddr*)&client, &clientAddrLen);
  if (socket == INVALID_SOCKET) {
    LOG_FATAL_SOCK_ERROR();
    return nullptr;
  }

	// C4:  Good case for a private constructor, though just doing it here is fine; 
  TCPSocket* sock = new TCPSocket(socket, (sockaddr&)client);

  return sock;
}



bool TCPSocket::connect(const NetAddress& addr) {

  sockaddr_storage saddr;
  int len;
  addr.toSockaddr((sockaddr&)saddr, len);

  SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  int result = ::connect(sock, (sockaddr*)&saddr, len);


  if(result == SOCKET_ERROR) {
    LOG_FATAL_SOCK_ERROR();
    close();
    Log::tagf("net", "could not connect %s", addr.toString());
    return false;
  }

  mHandle = sock;
  mAddress.fromSockaddr((sockaddr&)saddr);
  Log::tagf("net", "connected to %s", addr.toString());
  return true;
}

size_t TCPSocket::send(const void* data, size_t size) {

  if (closed()) return 0;

  int result = ::send(mHandle, (char*)data, (int)size, 0);

  if(result < 0) {
    bool re;
    OUT_LOG_FATAL_SOCK_ERROR(re);

    if(re) {
      close();
      return 0;
    }
  }

  if(result != (int)size) {
    Log::tagf("net", "the send data size is: %i, but the expected size is: %u", result, size);
    return 0;
  }

  return result;
}

size_t TCPSocket::receive(void* buf, size_t maxSize) {

  int re = ::recv(mHandle, (char*)buf, (int)maxSize, 0);


  if(re == 0) {
    close();
  } else if(re == SOCKET_ERROR) {
    bool rre;

    OUT_LOG_FATAL_SOCK_ERROR(rre);

    if(rre) { 
      close();
      Log::tagf("net", "Receive data failed");
      return 0;
    }
  }

  ENSURES(re <= int(maxSize));

  return re == -1 ? 0 : re;
}

bool TCPSocket::bind(const NetAddress& addr, uint16_t /*portRange*/) {

  ASSERT_OR_RETURN(!opened(), false);

  SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  ASSERT_OR_RETURN(sock != INVALID_SOCKET, false);

  sockaddr_storage sockadd;
  int len = sizeof(sockaddr_storage);
  addr.toSockaddr((sockaddr&)sockadd, len);

  int result = ::bind(sock, (sockaddr*)&sockadd, len);

  if (0 == result) {
    mHandle = sock;
    mAddress = addr;
    return true;
  }

  return false;
}


bool TCPSocket::listen(uint16_t port, uint maxQueued) {
  NetAddress local = NetAddress::any(port);

  bool re = bind(local);

  if (!re) return false;

  int result = ::listen(mHandle, maxQueued);
  if (result != 0) {
    bool rer;
    OUT_LOG_FATAL_SOCK_ERROR(rer);
    if (rer) {
      close();
      Log::tagf("net", "fail to listen at %u", port);
      return false;
    }
  }

  Log::tagf("net", "start listening at %u", port);

  return true;
}
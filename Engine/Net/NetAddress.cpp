
#include "NetAddress.hpp"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Debug/Console/Console.hpp"

bool addr(sockaddr* out, int* outLen, const char* name, const char* service = "80") {
  bool resolved = false;
  if (name[0] == '\0') {
    return false;
  }

  addrinfo hints;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  // hints.ai_flags = AI_PASSIVE;

  addrinfo* result = nullptr;
  int status = ::getaddrinfo(name, service, &hints, &result);

  if (status != 0) {
    Log::tagf("net", "failed to find address for \\[%s:%s]. Error \\[%s]", name, service, ::gai_strerror(status));
    return false;
  }

  addrinfo* iter = result;

  while (iter != nullptr) {
    if (iter->ai_family == AF_INET) {
      sockaddr_in* ipv4 = (sockaddr_in*)(iter->ai_addr);
      memcpy(out, ipv4, sizeof(sockaddr_in));
      *outLen = sizeof(sockaddr_in);
      resolved = true;
    }

    iter = iter->ai_next;
  }

  ::freeaddrinfo(result);
  return true;
}

NetAddress::NetAddress(const sockaddr& addr) {
  fromSockaddr(addr);
}

NetAddress::NetAddress(std::string_view str) {
  sockaddr_storage sock;
  int len;

  auto address = split(str.data(), ":");
  addr((sockaddr*)&sock, &len, address[0].c_str(), address[1].c_str());
  fromSockaddr((const sockaddr&)sock);
}

bool NetAddress::toSockaddr(sockaddr& outAddr, int& outLen) const {
  sockaddr_in& ipv4 = (sockaddr_in&)outAddr;

  ipv4.sin_family = AF_INET;
  ipv4.sin_addr.S_un.S_addr = mIpv4Address;
  ipv4.sin_port = ::htons(mPort);

  outLen = sizeof(sockaddr_in);
  return true;
}

bool NetAddress::fromSockaddr(const sockaddr& addr) {
  sockaddr_in& ipv4 = (sockaddr_in&)addr;

  mIpv4Address = ipv4.sin_addr.S_un.S_addr;
  mPort = ntohs(ipv4.sin_port);

  char buf[256];
  buf[0] = 0;

  inet_ntop(ipv4.sin_family, &ipv4.sin_addr, buf, 256);
  sprintf_s(mNameCache, "%s:%u", buf, mPort);
  return true;
}

 NetAddress NetAddress::local(uint16_t port) {
  char name[256];

  if (SOCKET_ERROR == ::gethostname(name, 256)) {
    return NetAddress();
  }

  char const *service = "80";

  if (name[0] == '\0') {
    return NetAddress();;
  }

  addrinfo hints;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  addrinfo* result = nullptr;
  int status = ::getaddrinfo(name, service, &hints, &result);

  if (status != 0) {
    Log::tagf("net", "failed to find address for \\[%s:%s]. Error \\[%s]", name, service, ::gai_strerror(status));
    return NetAddress();
  }

  addrinfo* iter = result;

  while (iter != nullptr) {
    if (iter->ai_family == AF_INET) {
      sockaddr_in* ipv4 = (sockaddr_in*)(iter->ai_addr);
      ipv4->sin_port = ::htons(port);

      NetAddress addr;
      addr.fromSockaddr((sockaddr&)*ipv4);
    
      ::freeaddrinfo(result);
  
      return addr;
      // char out[256];
      // ::inet_ntop(ipv4->sin_family, &(ipv4->sin_addr), out, 256);
      // Log::tagf("net", "Address: %s", out);
    }

    iter = iter->ai_next;
  }

  ::freeaddrinfo(result);
  return NetAddress();
}

NetAddress NetAddress::any(uint16_t port) {
  NetAddress addr;

  sockaddr_in ipv4;
  ipv4.sin_family = AF_INET;
  ipv4.sin_addr.S_un.S_addr = INADDR_ANY;
  ipv4.sin_port = ::htons(port);

  addr.fromSockaddr((sockaddr&)ipv4);
  return addr;
}

const char* NetAddress::toString() const {
  return mNameCache;
}

bool NetAddress::operator==(const NetAddress& addr) const {
  return
    (addr.mIpv4Address == mIpv4Address) &&
    (addr.mPort == mPort);
}

bool NetAddress::operator!=(const NetAddress& addr) const {
  return !(*this == addr);
}

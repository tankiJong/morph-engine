#pragma once
#include "Engine/Core/common.hpp"

struct sockaddr;
class NetAddress {
public:
  NetAddress() = default;
  NetAddress(const sockaddr& addr);
  NetAddress(std::string_view str);

  bool toSockaddr(sockaddr& outAddr, int& outLen) const;
  bool fromSockaddr(const sockaddr& addr);
  bool valid() const { return mPort != 0; }
  static NetAddress local(uint16_t port = 80);
  static NetAddress any(uint16_t port = 80);
  const char* toString() const;
protected:
  uint32_t mIpv4Address = 0;
  uint16_t mPort = 0;
  char mNameCache[256];
};

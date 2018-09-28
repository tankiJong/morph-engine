#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/Socket.hpp"

class UDPSocket: public Socket {
public:
  UDPSocket() = default;

  UDPSocket(UDPSocket&& socket) = default;
  UDPSocket& operator=(UDPSocket&& rhs) = default;
  UDPSocket(UDPSocket& copy) = delete;
  UDPSocket& operator=(const UDPSocket& rhs) = delete;

  bool bind(const NetAddress& addr, uint16_t portRange = 0u);

  size_t send(const NetAddress& addr, const void* data, size_t byteCount);

  size_t receive(NetAddress& outAddr, void* buffer, size_t maxSize);
};

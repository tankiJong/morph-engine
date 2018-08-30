#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/NetAddress.hpp"

class NetAddress;

class TCPSocket {
public:
  TCPSocket();
  ~TCPSocket();

  owner<TCPSocket*> accept();
  bool bind(const NetAddress& addr);

  bool connect(const NetAddress& addr);
  bool listen(uint16_t port, uint maxQueued = 16);

  void send(void* data, size_t size);
  size_t receive(void* buf, size_t max = 65536);
  void close();

  bool isClosed();

  const NetAddress& address() const { return mAddress; }

  bool valid() const;
protected:
  uintptr_t mHandle;
  NetAddress mAddress;
};

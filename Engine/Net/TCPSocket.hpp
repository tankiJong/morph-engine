#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/Socket.hpp"
class NetAddress;

class TCPSocket {
public:
  TCPSocket();

  TCPSocket(TCPSocket&& socket);
  TCPSocket& operator=(TCPSocket&& rhs);
  TCPSocket(TCPSocket& copy) = delete;
  TCPSocket& operator=(const TCPSocket& rhs) = delete;

  ~TCPSocket();

  owner<TCPSocket*> accept();
  bool bind(const NetAddress& addr);

  bool connect(const NetAddress& addr);
  bool listen(uint16_t port, uint maxQueued = 16);

  void send(const void* data, size_t size);

  template<typename T>
  void send(T& data) {
    send(&data, sizeof(T));
  }

  size_t receive(void* buf, size_t max = 65536);
  void close();

  bool isClosed();

  const NetAddress& address() const { return mAddress; }

  bool valid() const;
protected:
  uintptr_t mHandle;
  NetAddress mAddress;
};

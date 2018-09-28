#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/Socket.hpp"
class NetAddress;

class TCPSocket: public Socket {
public:
  TCPSocket() = default;

  TCPSocket(TCPSocket&& socket) = default;
  TCPSocket& operator=(TCPSocket&& rhs) = default;
  TCPSocket(TCPSocket& copy) = delete;
  TCPSocket& operator=(const TCPSocket& rhs) = delete;

  virtual ~TCPSocket() = default;;

	// C4: You use move semantics - don't need this - pass by value; 
  owner<TCPSocket*> accept();

  bool connect(const NetAddress& addr);
  bool listen(uint16_t port, uint maxQueued = 16);

  size_t send(const void* data, size_t size);

  template<typename T>
  void send(T& data) {
    send(&data, sizeof(T));
  }

  size_t receive(void* buf, size_t max = 65536);

protected:
  TCPSocket(uintptr_t handle, const NetAddress& addr): Socket(handle, addr) {}
  bool bind(const NetAddress& addr, uint16_t portRange = 0u);
};

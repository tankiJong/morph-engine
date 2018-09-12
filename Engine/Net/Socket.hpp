#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Net/NetAddress.hpp"

enum eSocketOption: uint {
  SOCKET_OPTION_BLOCKING = BIT_FLAG(0),

};

enum_class_operators(eSocketOption);

class Socket {
public:
  Socket();
  virtual ~Socket();

  void setOption(eSocketOption ops);
  void unsetOption(eSocketOption ops);

  bool close();

  bool valid() const;

  const NetAddress& address() const { return mAddress; };

protected:
  
  uintptr_t mHandle;
  NetAddress mAddress;

  eSocketOption mOptions;
};


#define LOG_FATAL_SOCK_ERROR() { \
int re = WSAGetLastError(); \
  if (re != WSAEWOULDBLOCK && re != WSAEMSGSIZE && re != WSAECONNRESET) { \
    Log::tagf("net", "Fatal error trigger in %s, code: %u", __FUNCTION__, re); \
  } \
} 

#define OUT_LOG_FATAL_SOCK_ERROR(rre) { \
  int ___re = WSAGetLastError(); \
  if (___re != WSAEWOULDBLOCK && ___re != WSAEMSGSIZE && ___re != WSAECONNRESET) { \
    Log::tagf("net", "Fatal error trigger in %s, code: %u", __FUNCTION__, ___re); \
    rre = true; \
  } \
  else { rre = false; }\
};
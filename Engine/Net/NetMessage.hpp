#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/BytePacker.hpp"

#define NET_MESSAGE_MTU 1 KB

class NetMessage: public BytePacker {
  friend class UDPSession;
  friend class NetPacket;
public:

  struct Def {
    static const uint8_t INVALID_MESSAGE_INDEX = ~0;
    uint8_t index;
    std::string name;
  };
  NetMessage()
    : BytePacker(NET_MESSAGE_MTU, mLocalBuffer, ENDIANNESS_LITTLE) {}

  NetMessage(uint index) 
    : BytePacker(NET_MESSAGE_MTU, mLocalBuffer, ENDIANNESS_LITTLE) 
    , mIndex(index) {}

  NetMessage(std::string_view name)
    : BytePacker(NET_MESSAGE_MTU, mLocalBuffer, ENDIANNESS_LITTLE)
    , mName(name) {};

  NetMessage(const NetMessage& msg);
  NetMessage& operator=(const NetMessage& rhs);
  const std::string& name() const { return mName; }
  uint8_t index() const { return mIndex; }
protected:

  void setDefinition(const Def& def);
  byte_t mLocalBuffer[NET_MESSAGE_MTU];
  const Def* mDefinition = nullptr;
  std::string mName = "";
  uint8_t mIndex = Def::INVALID_MESSAGE_INDEX;
};

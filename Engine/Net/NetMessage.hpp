#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/BytePacker.hpp"

#define NET_MESSAGE_MTU 1 KB

enum eMessageOption {
  NETMESSAGE_OPTION_CONNECTIONLESS = BIT_FLAG(0), // task14
  NETMESSAGE_OPTION_RELIABLE = BIT_FLAG(1), // task15
  NETMESSAGE_OPTION_IN_ORDER = BIT_FLAG(2), // task16

  // convenience
  NETMSSAGE_OPTION_RELIALBE_IN_ORDER = NETMESSAGE_OPTION_RELIABLE | NETMESSAGE_OPTION_IN_ORDER,
};

class NetMessage: public BytePacker {
  friend class UDPSession;
  friend class NetPacket;
public:
  static constexpr uint16_t INVALID_MESSAGE_RELIABLE_ID = 0xffui8;

  struct Def {
    static constexpr uint8_t INVALID_MESSAGE_INDEX = 0xffui8;
    uint8_t index = INVALID_MESSAGE_INDEX;
    std::string name;
    eMessageOption options;
  };


  NetMessage()
    : BytePacker(NET_MESSAGE_MTU, mLocalBuffer, ENDIANNESS_LITTLE) {}

  NetMessage(uint8_t index) 
    : BytePacker(NET_MESSAGE_MTU, mLocalBuffer, ENDIANNESS_LITTLE) 
    , mIndex(index) {}

  NetMessage(std::string_view name)
    : BytePacker(NET_MESSAGE_MTU, mLocalBuffer, ENDIANNESS_LITTLE)
    , mName(name) {}

  NetMessage(const NetMessage& msg);
  NetMessage& operator=(const NetMessage& rhs);
  const std::string& name() const { return mName; }
  uint8_t index() const { return mIndex; }

  bool reliable() const { return mReliableId < INVALID_MESSAGE_RELIABLE_ID; }
  uint16_t reliableId() const { return mReliableId; }
  bool connectionless() const;
protected:

  void setDefinition(const Def& def);
  byte_t mLocalBuffer[NET_MESSAGE_MTU] = {};
  const Def* mDefinition = nullptr;
  std::string mName = "";
  uint8_t mIndex = Def::INVALID_MESSAGE_INDEX;
  double mLastSendSec;
  uint16_t mReliableId = INVALID_MESSAGE_RELIABLE_ID;
};

enum_class_operators(eMessageOption);

#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/BytePacker.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

#define NET_MESSAGE_MTU 1 KB

enum eMessageOption {
  NETMESSAGE_OPTION_DEFAULT = 0, // unreliable
  NETMESSAGE_OPTION_CONNECTIONLESS = BIT_FLAG(0),
  NETMESSAGE_OPTION_RELIABLE = BIT_FLAG(1),
  NETMESSAGE_OPTION_IN_ORDER = BIT_FLAG(2),

  // convenience
  NETMSSAGE_OPTION_RELIALBE_IN_ORDER = NETMESSAGE_OPTION_RELIABLE | NETMESSAGE_OPTION_IN_ORDER,
};
enum_class_operators(eMessageOption);

class NetMessage: public BytePacker {
  friend class UDPSession;
  friend class NetPacket;
public:
  static constexpr uint16_t INVALID_MESSAGE_RELIABLE_ID = 0xffui8;

  struct Def {
    static constexpr uint8_t INVALID_MESSAGE_INDEX = 0xffui8;
    uint8_t index = INVALID_MESSAGE_INDEX;
    uint8_t channelIndex = 0;
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
  static uint8_t headerSize(bool reliable, bool inorder) {
    if(reliable) {
      return inorder ? 5 : 3;
    }

    EXPECTS(!inorder);
    return 1;
  }

  uint8_t headerSize() const { return headerSize(reliable(), inorder()); }
  bool reliable() const {
    return is_set(mDefinition->options, NETMESSAGE_OPTION_RELIABLE);
  }
  uint16_t reliableId() const { return mReliableId; }
  void reliableId(uint16_t id) { mReliableId = id; }

  void sequenceId(uint16_t id) { mSequenceId = id; }
  uint16_t sequenceId() const { return mSequenceId; }

  bool inorder() const { return is_set(mDefinition->options, NETMESSAGE_OPTION_IN_ORDER); }
  bool connectionless() const;

  double secondAfterLastSend() const;

  double& lastSendSec() { return mLastSendSec; }
  double lastSendSec() const { return mLastSendSec; }

  const Def* definition() const { return mDefinition; }
protected:

  void setDefinition(const Def& def);

  byte_t mLocalBuffer[NET_MESSAGE_MTU] = {};
  const Def* mDefinition = nullptr;
  std::string mName = "";
  uint8_t mIndex = Def::INVALID_MESSAGE_INDEX;
  double mLastSendSec;
  uint16_t mReliableId = INVALID_MESSAGE_RELIABLE_ID;
  uint16_t mSequenceId = 0;
};


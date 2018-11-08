#include "NetMessage.hpp"
#include "Engine/Core/Time/Time.hpp"

NetMessage::NetMessage(const NetMessage& msg)
  : NetMessage() {

  mNextWrite = msg.mNextWrite;
  mNextRead = msg.mNextRead;

  memcpy(mLocalBuffer, msg.mLocalBuffer, NET_MESSAGE_MTU);
  mDefinition = msg.mDefinition;
  mIndex = msg.mIndex;
  mName = msg.mName;
  mLastSendSec = msg.mLastSendSec;
  mReliableId = msg.mReliableId;
}

NetMessage& NetMessage::operator=(const NetMessage& rhs) {
  new(this)NetMessage(rhs);
  return *this;
}

bool NetMessage::connectionless() const {
  return is_set(mDefinition->options, NETMESSAGE_OPTION_CONNECTIONLESS);
}

double NetMessage::secondAfterLastSend() const {
  return GetCurrentTimeSeconds() - mLastSendSec;
}

void NetMessage::setDefinition(const Def& def) {
  mDefinition = &def;
  mName = mDefinition->name;
  mIndex = mDefinition->index;
}

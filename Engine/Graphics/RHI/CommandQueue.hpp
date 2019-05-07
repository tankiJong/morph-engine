#pragma once

#include "RHI.hpp"
#include "Engine/Core/common.hpp"

#include "Engine/Graphics/RHI/CommandList.hpp"

class CommandList;

class CommandQueue {
  using sptr_t = S<CommandQueue>;

  void execute(CommandList::sptr_t cmdlist);

  static sptr_t create(eCommandQueueType type);
protected:
  CommandQueue(eCommandQueueType type): mType(type) {}
  command_queue_handle_t mHandle;
  eCommandQueueType      mType;
};
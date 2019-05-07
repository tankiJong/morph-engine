#pragma once
#include "Engine/Core/common.hpp"

class CommandList {
public:
  using sptr_t = S<CommandList>;
  enum class eType {
    Direct,
    Compute,
    Copy,
  };
  sptr_t create(eType type);

public:
  eType type;
};

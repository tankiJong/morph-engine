#pragma once

#include "Engine/Core/common.hpp"

#pragma comment(lib, "ws2_32.lib")

class Net {
public:
  static bool startup();
  static bool shutdown();
};
#pragma once

#include "Engine/Core/common.hpp"

#pragma comment(lib, "ws2_32.lib")

#define ETHERNET_MTU 1500 

class Net {
public:
  static bool startup();
  static bool shutdown();
};
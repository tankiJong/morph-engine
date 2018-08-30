#pragma once

#include "Engine/Core/common.hpp"

#pragma comment(lib, "ws2_32.lib")

class Net {
public:
  static bool startup();
  static bool shutdown();
};

enum eEndianness {
  ENDIANNESS_BIG = 0,
  ENDIANNESS_LITTLE = 1,
};


#include "Engine/Core/Misc/Uuid.hpp"
#include <windows.h>

#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000

std::string getUuid() {
  UUID uuid;

  UuidCreate(&uuid);
  char* id;

  UuidToStringA(&uuid, (RPC_CSTR*)&id);

  std::string re(id);

  RpcStringFreeA((RPC_CSTR*)&id);
  return re;
}

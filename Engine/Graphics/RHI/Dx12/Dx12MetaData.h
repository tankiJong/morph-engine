#pragma once
#include "Engine/Graphics/RHI/FencedPool.hpp"
#include "Engine/Graphics/RHI/Dx12/dx12util.hpp"

struct RHIMetaData {
  FencedPool<command_alloctor_handle_t>::sptr_t allocatorPool;
};

#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIBuffer.hpp"

class ConstantBuffer: public RHIBuffer, public inherit_shared_from_this<RHIBuffer, ConstantBuffer> {
public:
  
};

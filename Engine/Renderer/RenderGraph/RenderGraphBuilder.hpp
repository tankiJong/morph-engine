#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/Program/ParamData.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"

class RenderGraphBuilder {
public:
  struct BindingInfo {
    RHIResource::scptr_t res;
    DescriptorSet::Type type;

    union {
      const ConstantBufferView* cbv;
      const ShaderResourceView* srv;
      const UnorderedAccessView* uav;
      const RenderTargetView* rtv;
      const DepthStencilView* dsv;
    };

    // bool isOwned;
  };

  // read
  void readSrv(RHIResource::scptr_t res, uint registerIndex, uint registerSpace = 0);
  void readCbv(RHIResource::scptr_t res, uint registerIndex, uint registerSpace = 0);

  // write
  void writeRtv(RHIResource::scptr_t res, uint registerIndex, uint registerSpace = 0);

  // read & write
  void readWriteUav(RHIResource::scptr_t tex, uint index);
};

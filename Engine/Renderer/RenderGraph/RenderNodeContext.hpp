#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/Program/ParamData.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Renderer/RenderGraph/RenderEdge.hpp"
#include "Engine/Graphics/RHI/PipelineState.hpp"
#include "Engine/Renderer/RenderGraph/RenderGraphResourceSet.hpp"
#include <variant>

class RenderNodeContext {
  friend class RenderNode;
public:
  RenderNodeContext(RenderNode* owner): mOwner(owner) { }
  ~RenderNodeContext();
  // read
  void readSrv(std::string_view name, uint registerIndex, uint registerSpace = 0);
  void readCbv(std::string_view name, uint registerIndex, uint registerSpace = 0);

  // write
  void writeRtv(std::string_view name, uint registerIndex = 0);
  void writeDsv(std::string_view name);

  // read & write
  void readWriteUav(std::string_view name, uint registerIndex, uint registerSpace = 0);

  void readWriteUav(const RenderResourceHandle& res, uint registerIndex, uint registerSpace = 0);
  void writeRtv(const RenderResourceHandle& res, uint registerIndex = 0);

  void reset(Program::scptr_t prog, bool forCompute = false);

  RenderEdge::BindingInfo* find(std::string_view name);
  bool exists(RenderEdge::BindingInfo* info) const;

  void build();
  void apply(RHIContext& ctx);

  // resource manipulation

  template<typename T, typename ResType = rhi_sptr_t<T>>
  RenderResourceHandle& bind(ResType&& rhiResource, std::string name) {
    name = resolveResourceName(name);

    RenderGraphResourceSet& set = resourceSet();

    RenderResourceHandle& handle = set.declare<T>(name);

    set.set<T>(rhiResource, handle, RenderGraphResourceHandle::OWN_EXTERNAL, true);

    return handle;
  };

  RenderResourceHandle& extend(const RenderResourceHandle& source, std::string name, const RenderGraphResourceDesc& desc);

  template<typename T>
  RenderResourceHandle& declare(std::string name) {
    name = resolveResourceName(name);
    return resourceSet().declare<T>(name);
  }


protected:

  RenderGraphResourceSet& resourceSet() const;

  void addParamBindingInfo(std::string_view param, RHIResource::State state, uint registerIndex, uint registerSpace);

  void addExternalBindingInfo(std::string_view resName, RHIResource::State state, uint registerIndex, uint registerSpace);

  void addBindingInfo(const RenderResourceHandle& resHandle, RHIResource::State state, uint registerIndex, uint registerSpace);

  void buildForCompute();
  void buildForGraphics();

  void bindRuntimeResource(const RHIResource& res, RenderEdge::BindingInfo& info);

  std::string resolveResourceName(std::string name) const;

  std::map<std::string, RenderEdge::BindingInfo, std::greater<>> mBindingInfos{};

  S<ProgramInst> mTargetProgram = nullptr;
  bool mForCompute = false;
  std::variant<std::monostate, GraphicsState::sptr_t, ComputeState::sptr_t> mPiplineState;
  FrameBuffer mFrameBuffer{};

  RenderNode* mOwner = nullptr;
};

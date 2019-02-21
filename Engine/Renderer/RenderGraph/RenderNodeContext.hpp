#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIResource.hpp"
#include "Engine/Graphics/Program/ParamData.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Renderer/RenderGraph/RenderEdge.hpp"
#include "Engine/Graphics/RHI/PipelineState.hpp"
#include <variant>

class RenderNodeContext {
public:
  RenderNodeContext(RenderNode* owner): mOwner(owner) { }
  ~RenderNodeContext();
  // read
  void readSrv(std::string_view name, RHIResource::scptr_t res, uint registerIndex, uint registerSpace = 0);
  void readCbv(std::string_view name, RHIResource::scptr_t res, uint registerIndex, uint registerSpace = 0);

  // write
  void writeRtv(std::string_view name, RHIResource::scptr_t res, uint registerIndex = 0);
  void writeDsv(std::string_view name, RHIResource::scptr_t res);

  // read & write
  void readWriteUav(std::string_view name, RHIResource::scptr_t tex, uint registerIndex, uint registerSpace = 0);


  void reset(Program::scptr_t prog, bool forCompute = false);

  RenderEdge::BindingInfo* find(std::string_view name);
  bool exists(RenderEdge::BindingInfo* info) const;

  void compile();
  void apply(RHIContext& ctx) const;

protected:

  void addBindingInfo(std::string_view name, RHIResource::scptr_t res, RHIResource::State state, uint registerIndex, uint registerSpace);

  void compileForCompute();
  void compileForGraphics();
  std::map<std::string, RenderEdge::BindingInfo, std::greater<>> mBindingInfos{};

  S<ProgramInst> mTargetProgram = nullptr;
  bool mForCompute = false;
  std::variant<std::monostate, GraphicsState::sptr_t, ComputeState::sptr_t> mPiplineState;
  FrameBuffer mFrameBuffer{};

  RenderNode* mOwner = nullptr;
};

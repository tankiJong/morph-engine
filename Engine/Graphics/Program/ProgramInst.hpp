#pragma once
#include <vector>
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Engine/Graphics/Program/ProgramData.hpp"

class Program;
class DescriptorSet;


class ProgramInst {
public:
  virtual ~ProgramInst() = default;

  void setCbv(const ConstantBufferView& cbv, uint registerIndex, uint registerSpace = 0);
  void setSrv(const ShaderResourceView& srv, uint registerIndex, uint registerSpace = 0);
  void setUav(const UnorderedAccessView& uav, uint registerIndex, uint registerSpace = 0);

  virtual bool apply(RHIContext& ctx, bool bindRootSignature) = 0;

  S<const Program> prog() const { return mProg; }

  // void set

protected:

  ProgramInst(const S<const Program>& program);

  S<const Program> mProg;
  ProgramData mProgramData;
};

class GraphicsProgramInst: public ProgramInst {
public:
  virtual bool apply(RHIContext& ctx, bool bindRootSignature) override;
  static S<GraphicsProgramInst> create(const S<const Program>& program);

  GraphicsProgramInst(const S<const Program>& program): ProgramInst(program) {};
};

class ComputeProgramInst: public ProgramInst {
public:
  virtual bool apply(RHIContext& ctx, bool bindRootSignature) override;
  static S<ComputeProgramInst> create(const S<Program>& program);

  ComputeProgramInst(const S<const Program>& program): ProgramInst(program) {};
};
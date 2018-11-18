#pragma once
#include <vector>
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/ResourceView.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/RHI/DescriptorPool.hpp"

class Program;
class DescriptorSet;


class ProgramIns {
public:
  virtual ~ProgramIns() = default;

  void setCbv(const ConstantBufferView& cbv, uint registerIndex, uint registerSpace = 0);
  void setSrv(const ShaderResourceView& srv, uint registerIndex, uint registerSpace = 0);
  void setUav(const UnorderedAccessView& uav, uint registerIndex, uint registerSpace = 0);

  virtual void apply(RHIContext& ctx, bool bindRootSignature) = 0;

  S<const Program> prog() const { return mProg; }

  // void set

protected:
  struct loc_t {
    uint descriptorSetIndex;
    size_t rangeIndex;
    uint zeroOffset;
  };

  ProgramIns(const S<Program>& program);

  static constexpr uint INVALID_SET_INDEX = UINT_MAX;
  loc_t locateBindPoint(DescriptorPool::Type type, uint registerIndex, uint registerSpace = 0) const;

  S<const Program> mProg;
  std::vector<S<DescriptorSet>> mDescriptorSets;
};

class GraphicsProgramIns: public ProgramIns {
public:
  virtual void apply(RHIContext& ctx, bool bindRootSignature) override;
  static S<GraphicsProgramIns> create(const S<Program>& program);

  GraphicsProgramIns(const S<Program>& program): ProgramIns(program) {};
};

class ComputeProgramIns: public ProgramIns {
public:
  virtual void apply(RHIContext& ctx, bool bindRootSignature) override;
  static S<ComputeProgramIns> create(const S<Program>& program);

  ComputeProgramIns(const S<Program>& program): ProgramIns(program) {};
};
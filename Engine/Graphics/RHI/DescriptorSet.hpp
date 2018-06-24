#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/Shader/Shader.hpp"
#include "Engine/Graphics/RHI/DescriptorPool.hpp"

class DescriptorSetRhiData;

enum class ShaderVisibility {
  None = 0,
  Vertex = (1 << (uint)eShaderType::SHADER_TYPE_VERTEX),
  Pixel = (1 << (uint)eShaderType::SHADER_TYPE_FRAGMENT),
  // Hull = (1 << (uint)eShaderType::Hull),
  // Domain = (1 << (uint)eShaderType::Domain),
  // Geometry = (1 << (uint)eShaderType::Geometry),
  // Compute = (1 << (uint)eShaderType::Compute),

  All = (1 << (uint)eShaderType::NUM_SHADER_TYPE) - 1,

};

enum_class_operators(ShaderVisibility);

class DescriptorSet {
public:
  using sptr_t = S<DescriptorSet>;
  using Type = DescriptorPool::Type;
  using rhi_handle_t = descriptor_set_rhi_handle_t;

  class Layout {
  public:
    struct Range {
      Type type;
      uint baseRegisterIndex;
      uint descCount;
      uint registerSpace;
    };

    Layout(ShaderVisibility visible = ShaderVisibility::All): mVisibility(visible) {}

    Layout& addRange(Type type, uint baseRegisterIndex, uint descriptorCount, uint registerSpace);

    size_t rangeCount() const { return mRanges.size(); }
    const Range& range(size_t index) const { return mRanges[index]; }
    ShaderVisibility visibility() const { return mVisibility; }
  protected:
    std::vector<Range> mRanges;
    ShaderVisibility mVisibility;
  };

  size_t rangeCount() const { return mLayout.rangeCount(); }
  const Layout::Range& range(uint index) const { return mLayout.range(index); }
  ShaderVisibility visibility() const { return mLayout.visibility(); }

  heap_cpu_handle_t cpuHandle(uint rangeIndex, uint offset = 0);
  heap_gpu_handle_t gpuHandle(uint rangeIndex, uint offset = 0);

  static sptr_t create(const DescriptorPool::sptr_t& pool, const Layout& layout);

protected:
  bool rhiInit();
  DescriptorSet(DescriptorPool::sptr_t pool, const Layout& layout): mPool(pool), mLayout(layout) {}
  DescriptorPool::sptr_t mPool;
  Layout mLayout;
  std::shared_ptr<DescriptorSetRhiData> mRhiData;
};
